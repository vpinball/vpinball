// Surface.cpp : Implementation of Surface
#include "StdAfx.h"

//////////////////////////////////////////////////////////////////
// Surface
Surface::Surface()
{
   m_menuid = IDR_SURFACEMENU;

   m_phitdrop = NULL;
   m_d.m_fCollidable = fTrue;
   m_d.m_fSlingshotAnimation = fTrue;
   m_d.m_fInner = fTrue;
   m_d.m_fEnableLighting = fTrue;
   slingshotVBuffer=0;
   sideVBuffer = 0;
   topVBuffer[0] = 0;
   topVBuffer[1] = 0;
}

Surface::~Surface()
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
   if( topVBuffer[0] )
   {
      topVBuffer[0]->release();
      topVBuffer[0]=0;
   }
   if( topVBuffer[1] )
   {
      topVBuffer[1]->release();
      topVBuffer[1]=0;
   }
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

   int iTmp;
   hr = GetRegInt("DefaultProps\\Wall", "EnableLighting", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fEnableLighting = (iTmp == 0) ? false : true;
   else
      m_d.m_fEnableLighting = fTrue;

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
   char strTmp[40];
   char strKeyName[20];
   strcpy_s(strKeyName, 20, IsWall ? "DefaultProps\\Wall" : "DefaultProps\\Target");

   SetRegValue(strKeyName,"TimerEnabled", REG_DWORD, &m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue(strKeyName,"TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue(strKeyName,"HitEvent", REG_DWORD, &m_d.m_fHitEvent,4);
   sprintf_s(strTmp, 40, "%f", m_d.m_threshold);
   SetRegValue(strKeyName,"HitThreshold", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_slingshot_threshold);
   SetRegValue(strKeyName,"SlingshotThreshold", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue(strKeyName,"SideColor", REG_DWORD, &m_d.m_sidecolor, 4);
   SetRegValue(strKeyName,"TopImage", REG_SZ, &m_d.m_szImage, strlen(m_d.m_szImage));
   SetRegValue(strKeyName,"SideImage", REG_SZ, &m_d.m_szSideImage, strlen(m_d.m_szImage));
   SetRegValue(strKeyName,"SlingshotColor", REG_DWORD, &m_d.m_slingshotColor, 4);
   SetRegValue(strKeyName,"TopColor", REG_DWORD, &m_d.m_topcolor, 4);
   SetRegValue(strKeyName,"Droppable", REG_DWORD, &m_d.m_fDroppable,4);
   SetRegValue(strKeyName,"Flipbook", REG_DWORD, &m_d.m_fFlipbook,4);
   SetRegValue(strKeyName,"CastsShadow", REG_DWORD, &m_d.m_fCastsShadow,4);
   sprintf_s(strTmp, 40, "%f", m_d.m_heightbottom);
   SetRegValue(strKeyName,"HeightBottom", REG_SZ, &strTmp, strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_heighttop);
   SetRegValue(strKeyName,"HeightTop", REG_SZ, &strTmp, strlen(strTmp));
   SetRegValue(strKeyName,"DisplayTexture", REG_DWORD, &m_d.m_fDisplayTexture,4);
   sprintf_s(strTmp, 40, "%f", m_d.m_slingshotforce);
   SetRegValue(strKeyName,"SlingshotForce", REG_SZ, &strTmp, strlen(strTmp));
   SetRegValue(strKeyName,"SlingshotAnimation", REG_DWORD, &m_d.m_fSlingshotAnimation,4);
   sprintf_s(strTmp, 40, "%f", m_d.m_elasticity);
   SetRegValue(strKeyName,"Elasticity", REG_SZ, &strTmp, strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_friction);
   SetRegValue(strKeyName,"Friction", REG_SZ, &strTmp, strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_scatter);
   SetRegValue(strKeyName,"Scatter", REG_SZ, &strTmp, strlen(strTmp));
   SetRegValue(strKeyName,"Visible", REG_DWORD, &m_d.m_fVisible,4);
   SetRegValue(strKeyName,"SideVisible", REG_DWORD, &m_d.m_fSideVisible,4);
   SetRegValue(strKeyName,"Collidable", REG_DWORD, &m_d.m_fCollidable,4);
   SetRegValue(strKeyName,"EnableLighting", REG_DWORD, &m_d.m_fEnableLighting,4);
}


HRESULT Surface::InitTarget(PinTable * const ptable, const float x, const float y, bool fromMouseClick)
{
   m_ptable = ptable;
   IsWall = false;
   float width = 30.0f, length=6.0f, fTmp;
   int iTmp;

   HRESULT hr = GetRegStringAsFloat("DefaultProps\\Target", "Width", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      width = fTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Target", "Length", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      length = fTmp;

   hr = GetRegInt("DefaultProps\\Target", "EnableLighting", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fEnableLighting = (iTmp == 0) ? false : true;
   else
      m_d.m_fEnableLighting = fTrue;

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

   hr = GetRegInt("DefaultProps\\Target","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = (iTmp == 0) ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = fFalse;

   hr = GetRegInt("DefaultProps\\Target","TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegInt("DefaultProps\\Target","HitEvent", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fHitEvent = (iTmp == 0) ? false : true;
   else
      m_d.m_fHitEvent = fTrue;

   hr = GetRegStringAsFloat("DefaultProps\\Target","HitThreshold", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_threshold = fTmp;
   else
      m_d.m_threshold = 2.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Target","SlingshotThreshold", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_slingshot_threshold = fTmp;
   else
      m_d.m_slingshot_threshold = 0.0f;

   //!! Deprecated, do not use anymore
   m_d.m_fInner = fTrue;

   hr = GetRegInt("DefaultProps\\Target","SideColor", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_sidecolor = iTmp;
   else
      m_d.m_sidecolor = RGB(127,127,127);

   hr = GetRegString("DefaultProps\\Target","TopImage", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegString("DefaultProps\\Target","SideImage", m_d.m_szSideImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSideImage[0] = 0;

   hr = GetRegInt("DefaultProps\\Target","SlingshotColor", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_slingshotColor = iTmp;
   else
      m_d.m_slingshotColor = RGB(242,242,242);

   hr = GetRegInt("DefaultProps\\Target","TopColor", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_topcolor = iTmp;
   else
      m_d.m_topcolor = RGB(127,127,127);

   hr = GetRegInt("DefaultProps\\Target","Droppable", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fDroppable = (iTmp == 0) ? false : true;
   else
      m_d.m_fDroppable = fFalse;

   hr = GetRegInt("DefaultProps\\Target","Flipbook", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fFlipbook = (iTmp == 0) ? false : true;
   else
      m_d.m_fFlipbook = fFalse;

   hr = GetRegInt("DefaultProps\\Target","CastsShadow", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCastsShadow = (iTmp == 0) ? false : true;
   else
      m_d.m_fCastsShadow = fTrue;

   hr = GetRegStringAsFloat("DefaultProps\\Target","HeightBottom", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_heightbottom = fTmp;
   else
      m_d.m_heightbottom = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Target","HeightTop", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_heighttop = fTmp;
   else
      m_d.m_heighttop = 50.0f;

   hr = GetRegInt("DefaultProps\\Target","DisplayTexture", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fDisplayTexture = (iTmp == 0) ? false : true;
   else
      m_d.m_fDisplayTexture = fFalse;

   hr = GetRegStringAsFloat("DefaultProps\\Target","SlingshotForce", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_slingshotforce = fTmp;
   else
      m_d.m_slingshotforce = 80.0f;

   hr = GetRegInt("DefaultProps\\Target","SlingshotAnimation", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fSlingshotAnimation = (iTmp == 0) ? false : true;
   else
      m_d.m_fSlingshotAnimation = fTrue;

   hr = GetRegStringAsFloat("DefaultProps\\Target","Elasticity", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_elasticity = fTmp;
   else
      m_d.m_elasticity = 0.3f;

   hr = GetRegStringAsFloat("DefaultProps\\Target","Friction", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_friction = fTmp;
   else
      m_d.m_friction = 0;	//zero uses global value

   hr = GetRegStringAsFloat("DefaultProps\\Target","Scatter", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_scatter = fTmp;
   else
      m_d.m_scatter = 0;	//zero uses global value

   hr = GetRegInt("DefaultProps\\Target","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = (iTmp == 0) ? false : true;
   else
      m_d.m_fVisible = fTrue;

   hr = GetRegInt("DefaultProps\\Target","SideVisible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fSideVisible = (iTmp == 0) ? false : true;
   else
      m_d.m_fSideVisible = fTrue;

   hr = GetRegInt("DefaultProps\\Target","Collidable", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCollidable = (iTmp == 0) ? fFalse : fTrue;
   else
      m_d.m_fCollidable = fTrue;

   return InitVBA(fTrue, 0, NULL);
}

void Surface::SetDefaults(bool fromMouseClick)
{
   int iTmp;
   float fTmp;

   HRESULT hr = GetRegInt("DefaultProps\\Wall","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = (iTmp == 0) ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = fFalse;

   hr = GetRegInt("DefaultProps\\Wall","TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegInt("DefaultProps\\Wall","HitEvent", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fHitEvent = (iTmp == 0) ? false : true;
   else
      m_d.m_fHitEvent = fFalse;

   hr = GetRegStringAsFloat("DefaultProps\\Wall","HitThreshold", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_threshold = fTmp;
   else
      m_d.m_threshold = 2.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Wall","SlingshotThreshold", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_slingshot_threshold = fTmp;
   else
      m_d.m_slingshot_threshold = 0.0f;

   //!! Deprecated, do not use anymore
   m_d.m_fInner = fTrue;

   hr = GetRegInt("DefaultProps\\Wall","SideColor", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_sidecolor = iTmp;
   else
      m_d.m_sidecolor = RGB(255,255,255);

   hr = GetRegString("DefaultProps\\Wall","TopImage", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegString("DefaultProps\\Wall","SideImage", m_d.m_szSideImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSideImage[0] = 0;

   hr = GetRegInt("DefaultProps\\Wall","SlingshotColor", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_slingshotColor = iTmp;
   else
      m_d.m_slingshotColor = RGB(242,242,242);

   hr = GetRegInt("DefaultProps\\Wall","TopColor", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_topcolor = iTmp;
   else
      m_d.m_topcolor = RGB(63,63,63);

   hr = GetRegInt("DefaultProps\\Wall","Droppable", &iTmp);
   if ((hr == S_OK)  && fromMouseClick)
      m_d.m_fDroppable = (iTmp == 0) ? false : true;
   else
      m_d.m_fDroppable = fFalse;

   hr = GetRegInt("DefaultProps\\Wall","Flipbook", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fFlipbook = (iTmp == 0) ? false : true;
   else
      m_d.m_fFlipbook = fFalse;

   hr = GetRegInt("DefaultProps\\Wall","CastsShadow", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCastsShadow = (iTmp == 0) ? false : true;
   else
      m_d.m_fCastsShadow = fTrue;

   hr = GetRegStringAsFloat("DefaultProps\\Wall","HeightBottom", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_heightbottom = fTmp;
   else
      m_d.m_heightbottom = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Wall","HeightTop", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_heighttop = fTmp;
   else
      m_d.m_heighttop = 50.0f;

   hr = GetRegInt("DefaultProps\\Wall","DisplayTexture", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fDisplayTexture = (iTmp == 0) ? false : true;
   else
      m_d.m_fDisplayTexture = fFalse;

   hr = GetRegStringAsFloat("DefaultProps\\Wall","SlingshotForce", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_slingshotforce = fTmp;
   else
      m_d.m_slingshotforce = 80.0f;

   hr = GetRegInt("DefaultProps\\Wall","SlingshotAnimation", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fSlingshotAnimation = (iTmp == 0) ? false : true;
   else
      m_d.m_fSlingshotAnimation = fTrue;

   hr = GetRegStringAsFloat("DefaultProps\\Wall","Elasticity", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_elasticity = fTmp;
   else
      m_d.m_elasticity = 0.3f;

   hr = GetRegStringAsFloat("DefaultProps\\Wall","Friction", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_friction = fTmp;
   else
      m_d.m_friction = 0;	//zero uses global value

   hr = GetRegStringAsFloat("DefaultProps\\Wall","Scatter", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_scatter = fTmp;
   else
      m_d.m_scatter = 0;	//zero uses global value

   hr = GetRegInt("DefaultProps\\Wall","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = (iTmp == 0) ? false : true;
   else
      m_d.m_fVisible = fTrue;

   hr = GetRegInt("DefaultProps\\Wall","SideVisible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fSideVisible = (iTmp == 0) ? false : true;
   else
      m_d.m_fSideVisible = fTrue;

   hr = GetRegInt("DefaultProps\\Wall","Collidable", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCollidable = (iTmp == 0) ? false : true;
   else
      m_d.m_fCollidable = fTrue;
}


void Surface::PreRender(Sur * const psur)
{
   psur->SetFillColor(RGB(192,192,192));
   psur->SetObject(this);
   // Don't want border color to be over-ridden when selected - that will be drawn later
   psur->SetBorderColor(-1,false,0);

   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);

   PinImage *ppi;
   if (m_d.m_fDisplayTexture && (ppi = m_ptable->GetImage(m_d.m_szImage)))
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

   for (int i=0;i<vvertex.Size();i++) //!! keep for render()
      delete vvertex.ElementAt(i);
}

void Surface::Render(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetObject(this); // For selected formatting
   psur->SetObject(NULL);

   Vector<RenderVertex> vvertex; //!! check/reuse from prerender
   GetRgVertex(&vvertex);

   psur->Polygon(vvertex);

   for (int i=0;i<vvertex.Size();i++)
      delete vvertex.ElementAt(i);

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

   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);

   psur->Polygon(vvertex);

   for (int i=0;i<vvertex.Size();i++)
      delete vvertex.ElementAt(i);
}

void Surface::RenderShadow(ShadowSur * const psur, const float height)
{	
   if ( (!m_d.m_fCastsShadow) || (!m_ptable->m_fRenderShadows) )
      return;

   psur->SetFillColor(RGB(0,0,0));
   psur->SetBorderColor(-1,false,0);
   psur->SetObject(this); // For selected formatting
   psur->SetObject(NULL);

   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);

   psur->PolygonSkew(vvertex, m_d.m_heightbottom, m_d.m_heighttop);

   for (int i=0;i<vvertex.Size();i++)
      delete vvertex.ElementAt(i);
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
   {
      pvht->AddElement(pht);
   }
}

void Surface::GetHitShapes(Vector<HitObject> * const pvho)
{
   CurvesToShapes(pvho);

   m_fIsDropped = fFalse;
   m_fDisabled = fFalse;
}

void Surface::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Surface::CurvesToShapes(Vector<HitObject> * const pvho)
{
   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);

   const int count = vvertex.Size();
   Vertex3Ds * const rgv3D = new Vertex3Ds[count];

   for (int i=0;i<count;i++)
   {
      const RenderVertex * const pv1 = vvertex.ElementAt(i);

      rgv3D[i].x = pv1->x;
      rgv3D[i].y = pv1->y;
      rgv3D[i].z = m_d.m_heighttop;

      const RenderVertex * const pv2 = vvertex.ElementAt((i < count-1) ? (i+1) : 0);
      const RenderVertex * const pv3 = vvertex.ElementAt((i < count-2) ? (i+2) : (i+2-count));

      AddLine(pvho, pv2, pv3, pv1, pv2->fSlingshot);
   }

   for (int i=0;i<count;i++)
      delete vvertex.ElementAt(i);

   if (m_d.m_fDroppable)
   {
      // Special hit object that will allow us to animate the surface
      m_phitdrop = new Hit3DPolyDrop(rgv3D,count);
      m_phitdrop->m_pfe = (IFireEvents *)this;

      m_phitdrop->m_fVisible = fTrue;

      m_phitdrop->m_polydropanim.m_iframedesire = 0;

      pvho->AddElement(m_phitdrop);			

      m_vhoDrop.AddElement(m_phitdrop);	

      m_vhoCollidable.AddElement(m_phitdrop);
      m_phitdrop->m_fEnabled = m_d.m_fCollidable;
   }
   else
   {
      Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,count);
      ph3dpoly->m_pfe = (IFireEvents *)this;

      ph3dpoly->m_fVisible = fTrue;

      pvho->AddElement(ph3dpoly);

      m_vhoCollidable.AddElement(ph3dpoly);
      ph3dpoly->m_fEnabled = m_d.m_fCollidable;
   }
}

void Surface::AddLine(Vector<HitObject> * const pvho, const RenderVertex * const pv1, const RenderVertex * const pv2, const RenderVertex * const pv3, const bool fSlingshot)
{
   LineSeg *plineseg;

   if (!fSlingshot)
   {
      plineseg = new LineSeg();

      if (m_d.m_fHitEvent)
      {
         plineseg->m_pfe = (IFireEvents *)this;
         plineseg->m_threshold = m_d.m_threshold;
      }
      else
      {
         plineseg->m_pfe = NULL;
      }
   }
   else
   {
      LineSegSlingshot * const plinesling = new LineSegSlingshot();
      plineseg = (LineSeg *)plinesling;

      // Slingshots always have hit events
      plineseg->m_pfe = (IFireEvents *)this;
      plineseg->m_threshold = m_d.m_threshold;

      plinesling->m_force = m_d.m_slingshotforce;
      plinesling->m_psurface = this;

      m_vlinesling.AddElement(plinesling);
   }

   plineseg->m_rcHitRect.zlow = m_d.m_heightbottom;
   plineseg->m_rcHitRect.zhigh = m_d.m_heighttop;

   plineseg->v1.x = pv1->x;
   plineseg->v1.y = pv1->y;
   plineseg->v2.x = pv2->x;
   plineseg->v2.y = pv2->y;

   plineseg->m_elasticity = m_d.m_elasticity;
   plineseg->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
   plineseg->m_scatter = ANGTORAD(m_d.m_scatter);

   pvho->AddElement(plineseg);
   if (m_d.m_fDroppable)
   {
      m_vhoDrop.AddElement(plineseg);
   }

   m_vhoCollidable.AddElement(plineseg);
   plineseg->m_fEnabled = m_d.m_fCollidable;

   plineseg->CalcNormal();

   const Vertex2D vt1(pv1->x - pv2->x, pv1->y - pv2->y);
   const Vertex2D vt2(pv1->x - pv3->x, pv1->y - pv3->y);

   const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

   if (dot < 0.f) // Inside edges don't need joint hit-testing (dot == 0 continuous segments should mathematically never hit)
   {
      Joint * const pjoint = new Joint();

      if (m_d.m_fHitEvent)
      {
         pjoint->m_pfe = (IFireEvents *)this;
         pjoint->m_threshold = m_d.m_threshold;
      }
      else
      {
         pjoint->m_pfe = NULL;
      }

      pjoint->m_rcHitRect.zlow = m_d.m_heightbottom;
      pjoint->m_rcHitRect.zhigh = m_d.m_heighttop;

      pjoint->m_elasticity = m_d.m_elasticity;
      pjoint->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
      pjoint->m_scatter = ANGTORAD(m_d.m_scatter);

      pjoint->center.x = pv1->x;
      pjoint->center.y = pv1->y;
      pvho->AddElement(pjoint);
      if (m_d.m_fDroppable)
      {
         m_vhoDrop.AddElement(pjoint);
      }

      m_vhoCollidable.AddElement(pjoint);
      pjoint->m_fEnabled = m_d.m_fCollidable;

      // Set up line normal
      {
         const float inv_length = 1.0f/sqrtf(vt2.x * vt2.x + vt2.y * vt2.y);
         pjoint->normal.x = plineseg->normal.x - vt2.y * inv_length;
         pjoint->normal.y = plineseg->normal.y + vt2.x * inv_length;
      }

      // Set up line normal
      {
         const float inv_length = 1.0f/sqrtf(pjoint->normal.x * pjoint->normal.x + pjoint->normal.y * pjoint->normal.y);
         pjoint->normal.x *= inv_length;
         pjoint->normal.y *= inv_length;
      }
   }
}

void Surface::GetBoundingVertices(Vector<Vertex3Ds> * const pvvertex3D)
{
   const float top = m_d.m_heighttop;
   const float bottom = m_d.m_heightbottom;

   for (int i=0;i<8;i++)
   {
      Vertex3Ds * const pv = new Vertex3Ds();
      pv->x = i&1 ? m_ptable->m_right : m_ptable->m_left;
      pv->y = i&2 ? m_ptable->m_bottom : m_ptable->m_top;
      pv->z = i&4 ? top : bottom;
      pvvertex3D->AddElement(pv);
   }
}

void Surface::EndPlay()
{
   IEditable::EndPlay();

   if (m_phitdrop) // Failed Player Case
   {
      if (m_d.m_fDroppable)
      {
         for (int i=0;i<2;i++)
         {
            delete m_phitdrop->m_polydropanim.m_pobjframe[i];
         }
      }

      m_phitdrop = NULL;
   }

   m_vlinesling.RemoveAllElements();
   m_vhoDrop.RemoveAllElements();
   m_vhoCollidable.RemoveAllElements();
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


void Surface::PostRenderStatic(const RenderDevice* pd3dDevice)
{
}
void Surface::PrepareWallsAtHeight( RenderDevice* pd3dDevice )
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   PinImage * const pinSide = m_ptable->GetImage(m_d.m_szSideImage);
   float maxtuSide=1.0f, maxtvSide=1.0f;

   const float inv_width  = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
   const float inv_height = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);

   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);
   float *rgtexcoord = NULL;

   if (pinSide)
   {
      m_ptable->GetTVTU(pinSide, &maxtuSide, &maxtvSide);		
      GetTextureCoords(&vvertex, &rgtexcoord);
   }

   numVertices = vvertex.Size();
   Vertex2D * const rgnormal = new Vertex2D[numVertices];
   if(!m_d.m_fEnableLighting)
   {
      if( !sideVBuffer )
      {
         pd3dDevice->createVertexBuffer( numVertices*4, 0, MY_D3DFVF_NOLIGHTING_VERTEX, &sideVBuffer );
         NumVideoBytes += numVertices*4*sizeof(Vertex3D_NoLighting);
      }

      vertsNotLit = new Vertex3D_NoLighting[numVertices*4];
   }
   else
   {
      if( !sideVBuffer )
      {
         pd3dDevice->createVertexBuffer( numVertices*4, 0, MY_D3DFVF_VERTEX, &sideVBuffer );
         NumVideoBytes += numVertices*4*sizeof(Vertex3D);
      }
      verts = new Vertex3D[numVertices*4];
   }

   for (int i=0;i<numVertices;i++)
   {
      const RenderVertex * const pv1 = vvertex.ElementAt(i);
      const RenderVertex * const pv2 = vvertex.ElementAt((i < numVertices-1) ? (i+1) : 0);
      const float dx = pv1->x - pv2->x;
      const float dy = pv1->y - pv2->y;

      const float inv_len = 1.0f/sqrtf(dx*dx + dy*dy);

      rgnormal[i].x = dy*inv_len;
      rgnormal[i].y = dx*inv_len;
   }

   Vertex3D_NoLighting *noLightBuf[2];
   Vertex3D *texelBuf[2];
   if(!m_d.m_fEnableLighting)
   {
      sideVBuffer->lock(0,0,(void**)&noLightBuf[0], VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE );
   }
   else
   {
      sideVBuffer->lock(0,0,(void**)&texelBuf[0], VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE );
   }

   int offset=0;
   // Render side
   for (int i=0;i<numVertices;i++, offset+=4)
   {
      const RenderVertex * const pv1 = vvertex.ElementAt(i);
      const RenderVertex * const pv2 = vvertex.ElementAt((i < numVertices-1) ? (i+1) : 0);

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

      {
         const float inv_len = 1.0f/sqrtf(vnormal[0].x * vnormal[0].x + vnormal[0].y * vnormal[0].y);
         vnormal[0].x *= inv_len;
         vnormal[0].y *= inv_len;
      }
      {
         const float inv_len = 1.0f/sqrtf(vnormal[1].x * vnormal[1].x + vnormal[1].y * vnormal[1].y);
         vnormal[1].x *= inv_len;
         vnormal[1].y *= inv_len;
      }

      if(!m_d.m_fEnableLighting)
      {
         //Vertex3D_NoLighting rgv3D[4];
         vertsNotLit[offset].x=pv1->x;     vertsNotLit[offset].y=pv1->y;     vertsNotLit[offset].z=m_d.m_heightbottom;
         vertsNotLit[offset+1].x=pv1->x;   vertsNotLit[offset+1].y=pv1->y;   vertsNotLit[offset+1].z=m_d.m_heighttop;
         vertsNotLit[offset+2].x=pv2->x;   vertsNotLit[offset+2].y=pv2->y;   vertsNotLit[offset+2].z=m_d.m_heighttop;
         vertsNotLit[offset+3].x=pv2->x;   vertsNotLit[offset+3].y=pv2->y;   vertsNotLit[offset+3].z=m_d.m_heightbottom;

         if (pinSide)
         {
            vertsNotLit[offset].tu = rgtexcoord[i] * maxtuSide;
            vertsNotLit[offset].tv = maxtvSide;

            vertsNotLit[offset+1].tu = rgtexcoord[i] * maxtuSide;
            vertsNotLit[offset+1].tv = 0;

            vertsNotLit[offset+2].tu = rgtexcoord[c] * maxtuSide;
            vertsNotLit[offset+2].tv = 0;

            vertsNotLit[offset+3].tu = rgtexcoord[c] * maxtuSide;
            vertsNotLit[offset+3].tv = maxtvSide;
         }

         for (int l=offset;l<offset+2;l++)
         {
            vertsNotLit[l].color = m_d.m_sidecolor;
            vertsNotLit[l+2].color = m_d.m_sidecolor;
         }

         memcpy( &noLightBuf[0][offset], &vertsNotLit[offset], sizeof(Vertex3D_NoLighting)*4 );
      }
      else
      {
         //Vertex3D rgv3D[4];
         verts[offset].x=pv1->x;     verts[offset].y=pv1->y;     verts[offset].z=m_d.m_heightbottom;
         verts[offset+1].x=pv1->x;   verts[offset+1].y=pv1->y;   verts[offset+1].z=m_d.m_heighttop;
         verts[offset+2].x=pv2->x;   verts[offset+2].y=pv2->y;   verts[offset+2].z=m_d.m_heighttop;
         verts[offset+3].x=pv2->x;   verts[offset+3].y=pv2->y;    verts[offset+3].z=m_d.m_heightbottom;

         if (pinSide)
         {
            verts[offset].tu = rgtexcoord[i] * maxtuSide;
            verts[offset].tv = maxtvSide;

            verts[offset+1].tu = rgtexcoord[i] * maxtuSide;
            verts[offset+1].tv = 0;

            verts[offset+2].tu = rgtexcoord[c] * maxtuSide;
            verts[offset+2].tv = 0;

            verts[offset+3].tu = rgtexcoord[c] * maxtuSide;
            verts[offset+3].tv = maxtvSide;
         }

         ppin3d->m_lightproject.CalcCoordinates(&verts[offset],inv_width,inv_height);
         ppin3d->m_lightproject.CalcCoordinates(&verts[offset+1],inv_width,inv_height);
         ppin3d->m_lightproject.CalcCoordinates(&verts[offset+2],inv_width,inv_height);
         ppin3d->m_lightproject.CalcCoordinates(&verts[offset+3],inv_width,inv_height);

         for (int l=offset;l<offset+2;l++)
         {
            verts[l].nx = -vnormal[0].x;
            verts[l].ny = vnormal[0].y;
            verts[l].nz = 0;

            verts[l+2].nx = -vnormal[1].x;
            verts[l+2].ny = vnormal[1].y;
            verts[l+2].nz = 0;
         }

         memcpy( &texelBuf[0][offset], &verts[offset], sizeof(Vertex3D)*4 );
      }
   }
   sideVBuffer->unlock();

   // draw top
   delete[] rgnormal;
   SAFE_VECTOR_DELETE(rgtexcoord);
   if (m_d.m_fVisible)
   {
      Vector<void> vpoly;

      for (int i=0;i<numVertices;i++)
      {
         vpoly.AddElement((void *)i);
      }

      Vector<Triangle> vtri;
      PolygonToTriangles(vvertex, &vpoly, &vtri);

      PinImage * const pin = m_ptable->GetImage(m_d.m_szImage);
      float maxtu=1.0f, maxtv=1.0f;
      if (pin)
      {
         m_ptable->GetTVTU(pin, &maxtu, &maxtv);
      }
      const float heightNotDropped = m_d.m_heighttop;
      const float heightDropped = (m_d.m_heightbottom + 0.1f);

      const float inv_tablewidth = maxtu/(m_ptable->m_right - m_ptable->m_left);
      const float inv_tableheight = maxtv/(m_ptable->m_bottom - m_ptable->m_top);

      numPolys = vtri.Size();
      if( numPolys==0 )
      {         
         for (int i=0;i<numVertices;i++)
            delete vvertex.ElementAt(i);

         // no polys to render leave vertex buffer undefined 
         return;
      }

      if(!m_d.m_fEnableLighting)
      {
         if( !topVBuffer[0] && !topVBuffer[1] )
         {
            pd3dDevice->createVertexBuffer( numPolys*3, 0, MY_D3DFVF_NOLIGHTING_VERTEX, &topVBuffer[0] );
            NumVideoBytes += numPolys*3*sizeof(Vertex3D_NoLighting);
            pd3dDevice->createVertexBuffer( numPolys*3, 0, MY_D3DFVF_NOLIGHTING_VERTEX, &topVBuffer[1] );
            NumVideoBytes += numPolys*3*sizeof(Vertex3D_NoLighting);
         }
         vertsTopNotLit[0] = new Vertex3D_NoLighting[numPolys*3];
         vertsTopNotLit[1] = new Vertex3D_NoLighting[numPolys*3];
      }
      else
      {
         if( !topVBuffer[0] && !topVBuffer[1] )
         {
            pd3dDevice->createVertexBuffer( numPolys*3, 0, MY_D3DFVF_VERTEX, &topVBuffer[0] );
            NumVideoBytes += numPolys*3*sizeof(Vertex3D);
            pd3dDevice->createVertexBuffer( numPolys*3, 0, MY_D3DFVF_VERTEX, &topVBuffer[1] );
            NumVideoBytes += numPolys*3*sizeof(Vertex3D);
         }
         vertsTop[0] = new Vertex3D[numPolys*3];
         vertsTop[1] = new Vertex3D[numPolys*3];
      }
      
      if(!m_d.m_fEnableLighting)
      {
         topVBuffer[0]->lock(0,0,(void**)&noLightBuf[0], VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE );
         topVBuffer[1]->lock(0,0,(void**)&noLightBuf[1], VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE );
      }
      else
      {
         topVBuffer[0]->lock(0,0,(void**)&texelBuf[0], VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE );
         topVBuffer[1]->lock(0,0,(void**)&texelBuf[1], VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE );
      }

      offset=0;
      for (int i=0;i<vtri.Size();i++, offset+=3 )
      {
         const Triangle * const ptri = vtri.ElementAt(i);

         const RenderVertex * const pv0 = vvertex.ElementAt(ptri->a);
         const RenderVertex * const pv1 = vvertex.ElementAt(ptri->b);
         const RenderVertex * const pv2 = vvertex.ElementAt(ptri->c);

         if(!m_d.m_fEnableLighting)
         {
            //Vertex3D_NoLighting rgv3D[3];
            vertsTopNotLit[0][offset].x=pv0->x;   vertsTopNotLit[0][offset].y=pv0->y;   vertsTopNotLit[0][offset].z=heightNotDropped;
            vertsTopNotLit[0][offset+2].x=pv1->x;   vertsTopNotLit[0][offset+2].y=pv1->y;   vertsTopNotLit[0][offset+2].z=heightNotDropped;
            vertsTopNotLit[0][offset+1].x=pv2->x;   vertsTopNotLit[0][offset+1].y=pv2->y;   vertsTopNotLit[0][offset+1].z=heightNotDropped;

            vertsTopNotLit[0][offset].tu = vertsTopNotLit[0][offset].x *inv_tablewidth;
            vertsTopNotLit[0][offset].tv = vertsTopNotLit[0][offset].y *inv_tableheight;
            vertsTopNotLit[0][offset+1].tu = vertsTopNotLit[0][offset+1].x *inv_tablewidth;
            vertsTopNotLit[0][offset+1].tv = vertsTopNotLit[0][offset+1].y *inv_tableheight;
            vertsTopNotLit[0][offset+2].tu = vertsTopNotLit[0][offset+2].x *inv_tablewidth;
            vertsTopNotLit[0][offset+2].tv = vertsTopNotLit[0][offset+2].y *inv_tableheight;

            for (int l=offset;l<offset+3;l++)
               vertsTopNotLit[0][l].color = m_d.m_topcolor;
            memcpy( &vertsTopNotLit[1][offset], &vertsTopNotLit[0][offset], sizeof(Vertex3D_NoLighting)*3 );
            vertsTopNotLit[1][offset].z = heightDropped;
            vertsTopNotLit[1][offset+1].z = heightDropped;
            vertsTopNotLit[1][offset+2].z = heightDropped;

            memcpy( &noLightBuf[0][offset], &vertsTopNotLit[0][offset], sizeof(Vertex3D_NoLighting)*3);
            memcpy( &noLightBuf[1][offset], &vertsTopNotLit[1][offset], sizeof(Vertex3D_NoLighting)*3);
         }
         else
         {
            //Vertex3D rgv3D[3];
            vertsTop[0][offset].x=pv0->x;   vertsTop[0][offset].y=pv0->y;   vertsTop[0][offset].z=heightNotDropped;
            vertsTop[0][offset+2].x=pv1->x;   vertsTop[0][offset+2].y=pv1->y;   vertsTop[0][offset+2].z=heightNotDropped;
            vertsTop[0][offset+1].x=pv2->x;   vertsTop[0][offset+1].y=pv2->y;   vertsTop[0][offset+1].z=heightNotDropped;

            vertsTop[0][offset].tu = vertsTop[0][offset].x *inv_tablewidth;
            vertsTop[0][offset].tv = vertsTop[0][offset].y *inv_tableheight;
            vertsTop[0][offset+1].tu = vertsTop[0][offset+1].x *inv_tablewidth;
            vertsTop[0][offset+1].tv = vertsTop[0][offset+1].y *inv_tableheight;
            vertsTop[0][offset+2].tu = vertsTop[0][offset+2].x *inv_tablewidth;
            vertsTop[0][offset+2].tv = vertsTop[0][offset+2].y *inv_tableheight;

            memcpy( &vertsTop[1][offset], &vertsTop[0][offset], sizeof(Vertex3D)*3 );
            vertsTop[1][offset].z = heightDropped;
            vertsTop[1][offset+1].z = heightDropped;
            vertsTop[1][offset+2].z = heightDropped;
            ppin3d->m_lightproject.CalcCoordinates(&vertsTop[0][offset],inv_width,inv_height);
            ppin3d->m_lightproject.CalcCoordinates(&vertsTop[0][offset+1],inv_width,inv_height);
            ppin3d->m_lightproject.CalcCoordinates(&vertsTop[0][offset+2],inv_width,inv_height);

            ppin3d->m_lightproject.CalcCoordinates(&vertsTop[1][offset],inv_width,inv_height);
            ppin3d->m_lightproject.CalcCoordinates(&vertsTop[1][offset+1],inv_width,inv_height);
            ppin3d->m_lightproject.CalcCoordinates(&vertsTop[1][offset+2],inv_width,inv_height);

            for (int l=offset;l<offset+3;l++)
            {
               vertsTop[0][l].nx = 0;
               vertsTop[0][l].ny = 0;
               vertsTop[0][l].nz = -1.0f;
               vertsTop[1][l].nx = 0;
               vertsTop[1][l].ny = 0;
               vertsTop[1][l].nz = -1.0f;
            }
            memcpy( &texelBuf[0][offset], &vertsTop[0][offset], sizeof(Vertex3D)*3);
            memcpy( &texelBuf[1][offset], &vertsTop[1][offset], sizeof(Vertex3D)*3);
         }
         delete vtri.ElementAt(i);
      }
      topVBuffer[0]->unlock();
      topVBuffer[1]->unlock();
   }

   for (int i=0;i<numVertices;i++)
      delete vvertex.ElementAt(i);
}

static const WORD rgiSlingshot0[4] = {0,1,4,3};
static const WORD rgiSlingshot1[4] = {1,2,5,4};
static const WORD rgiSlingshot2[4] = {0,3,4,1};
static const WORD rgiSlingshot3[4] = {1,4,5,2};
static const WORD rgiSlingshot4[4] = {3,9,10,4};
static const WORD rgiSlingshot5[4] = {4,10,11,5};

void Surface::PrepareSlingshots( RenderDevice *pd3dDevice )
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const float slingbottom = (m_d.m_heighttop - m_d.m_heightbottom) * 0.2f + m_d.m_heightbottom;
   const float slingtop    = (m_d.m_heighttop - m_d.m_heightbottom) * 0.8f + m_d.m_heightbottom;

   const float inv_width  = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
   const float inv_height = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);

   Vertex3D *buf;
   slingshotVBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE);
   int offset=0;
   for (int i=0;i<m_vlinesling.Size();i++)
   {
      LineSegSlingshot * const plinesling = m_vlinesling.ElementAt(i);
      plinesling->m_slingshotanim.m_fAnimations = (m_d.m_fSlingshotAnimation != 0);
      ObjFrame * const pof = new ObjFrame();

      plinesling->m_slingshotanim.m_pobjframe = pof;		
      ppin3d->ClearExtents(&plinesling->m_slingshotanim.m_rcBounds, &plinesling->m_slingshotanim.m_znear, &plinesling->m_slingshotanim.m_zfar);

      Vertex3D rgv3D[12];
      rgv3D[0].x = plinesling->v1.x;
      rgv3D[0].y = plinesling->v1.y;
      rgv3D[0].z = slingbottom;

      rgv3D[1].x = (plinesling->v1.x + plinesling->v2.x)*0.5f + plinesling->normal.x*(m_d.m_slingshotforce * 0.25f);//40;//20;
      rgv3D[1].y = (plinesling->v1.y + plinesling->v2.y)*0.5f + plinesling->normal.y*(m_d.m_slingshotforce * 0.25f);//20;
      rgv3D[1].z = slingbottom;

      rgv3D[2].x = plinesling->v2.x;
      rgv3D[2].y = plinesling->v2.y;
      rgv3D[2].z = slingbottom;

      for (int l=0;l<3;l++)
      {
         rgv3D[l+3].x = rgv3D[l].x;
         rgv3D[l+3].y = rgv3D[l].y;
         rgv3D[l+3].z = slingtop;
      }

      for (int l=0;l<6;l++)
      {
         rgv3D[l+6].x = rgv3D[l].x - plinesling->normal.x*5.0f;
         rgv3D[l+6].y = rgv3D[l].y - plinesling->normal.y*5.0f;
         rgv3D[l+6].z = rgv3D[l].z;
      }

      for (int l=0;l<12;l++)
      {
         ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l],inv_width,inv_height);
      }
      ppin3d->ClearExtents(&pof->rc, NULL, NULL);
      ppin3d->ExpandExtents(&pof->rc, rgv3D, &plinesling->m_slingshotanim.m_znear, &plinesling->m_slingshotanim.m_zfar, 6, fFalse);

      SetNormal(rgv3D, rgiSlingshot0, 4, NULL, NULL, NULL);
      memcpy( &buf[offset], &rgv3D[rgiSlingshot0[0]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot0[1]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot0[2]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot0[3]], sizeof(Vertex3D));
      offset++;

      SetNormal(rgv3D, rgiSlingshot1, 4, NULL, NULL, NULL);
      memcpy( &buf[offset], &rgv3D[rgiSlingshot1[0]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot1[1]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot1[2]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot1[3]], sizeof(Vertex3D));
      offset++;

      SetNormal(rgv3D, rgiSlingshot2, 4, NULL, NULL, NULL);
      memcpy( &buf[offset], &rgv3D[rgiSlingshot2[0]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot2[1]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot2[2]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot2[3]], sizeof(Vertex3D));
      offset++;

      SetNormal(rgv3D, rgiSlingshot3, 4, NULL, NULL, NULL);
      memcpy( &buf[offset], &rgv3D[rgiSlingshot3[0]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot3[1]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot3[2]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot3[3]], sizeof(Vertex3D));
      offset++;

      SetNormal(rgv3D, rgiSlingshot4, 4, NULL, NULL, NULL);
      memcpy( &buf[offset], &rgv3D[rgiSlingshot4[0]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot4[1]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot4[2]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot4[3]], sizeof(Vertex3D));
      offset++;

      SetNormal(rgv3D, rgiSlingshot5, 4, NULL, NULL, NULL);
      memcpy( &buf[offset], &rgv3D[rgiSlingshot5[0]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot5[1]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot5[2]], sizeof(Vertex3D));
      offset++;
      memcpy( &buf[offset], &rgv3D[rgiSlingshot5[3]], sizeof(Vertex3D));
      offset++;
   }
   slingshotVBuffer->unlock();
}

void Surface::RenderSetup(const RenderDevice* _pd3dDevice)
{
   RenderDevice *pd3dDevice = (RenderDevice*)_pd3dDevice;
   if( m_vlinesling.Size()>0 )
   {
      if( !slingshotVBuffer )
      {
         pd3dDevice->createVertexBuffer(m_vlinesling.Size()*24, 0, MY_D3DFVF_VERTEX, &slingshotVBuffer);
         NumVideoBytes += m_vlinesling.Size()*24*sizeof(Vertex3D);
      }
      PrepareSlingshots(pd3dDevice);
   }
   // create all vertices for dropped and non-dropped surface
   PrepareWallsAtHeight( pd3dDevice );
}

void Surface::RenderStatic(const RenderDevice* pd3dDevice)
{
   if (!m_d.m_fDroppable)
   {
      RenderWallsAtHeight( (RenderDevice*)pd3dDevice, fFalse, fFalse);
   }
}

void Surface::RenderSlingshots(RenderDevice* pd3dDevice)
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const float slingbottom = (m_d.m_heighttop - m_d.m_heightbottom) * 0.2f + m_d.m_heightbottom;
   const float slingtop    = (m_d.m_heighttop - m_d.m_heightbottom) * 0.8f + m_d.m_heightbottom;

   const float inv_width  = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
   const float inv_height = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);

   int offset=0;
   for (int i=0;i<m_vlinesling.Size();i++)
   {
      LineSegSlingshot * const plinesling = m_vlinesling.ElementAt(i);

      pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L );

      ppin3d->m_pddsZBuffer->Blt(NULL, ppin3d->m_pddsStaticZ, NULL, DDBLT_WAIT, NULL);

      ppin3d->SetMaterial( 1.0f, m_d.m_slingshotColor);
      ObjFrame *pof = plinesling->m_slingshotanim.m_pobjframe;

      pof->pdds = ppin3d->CreateOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
      pof->pddsZBuffer = ppin3d->CreateZBufferOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);

      pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, slingshotVBuffer, offset, 4, (LPWORD)rgi0123, 4, 0 );
      offset+=4;

      pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, slingshotVBuffer, offset, 4, (LPWORD)rgi0123, 4, 0 );
      offset+=4;

      pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, slingshotVBuffer, offset, 4, (LPWORD)rgi0123, 4, 0 );
      offset+=4;

      pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, slingshotVBuffer, offset, 4, (LPWORD)rgi0123, 4, 0 );
      offset+=4;

      pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, slingshotVBuffer, offset, 4, (LPWORD)rgi0123, 4, 0 );
      offset+=4;

      pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, slingshotVBuffer, offset, 4, (LPWORD)rgi0123, 4, 0 );
      offset+=4;

      pof->pdds->BltFast(0, 0, ppin3d->m_pddsBackBuffer, &pof->rc, DDBLTFAST_WAIT);
      pof->pddsZBuffer->BltFast(0, 0, ppin3d->m_pddsZBuffer, &pof->rc, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);

      ppin3d->ExpandRectByRect(&plinesling->m_slingshotanim.m_rcBounds, &pof->rc);

      // reset the portion of the z-buffer that we changed
      ppin3d->m_pddsZBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
      // Reset color key in back buffer
      DDBLTFX ddbltfx;
      ZeroMemory(&ddbltfx,sizeof(DDBLTFX));
      ddbltfx.dwSize = sizeof(DDBLTFX);
      ddbltfx.dwFillColor = 0;
      ppin3d->m_pddsBackBuffer->Blt(&pof->rc, NULL,&pof->rc, DDBLT_COLORFILL | DDBLT_WAIT,&ddbltfx);
   }
}

ObjFrame *Surface::RenderWallsAtHeight( RenderDevice* pd3dDevice, BOOL fMover, BOOL fDrop)
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   ppin3d->m_pddsZBuffer->Blt(NULL, ppin3d->m_pddsStaticZ, NULL, DDBLT_WAIT, NULL);

   if(!m_d.m_fEnableLighting)
      pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE);

   ObjFrame *pof = NULL;
   if (fMover)
   {
      pof = new ObjFrame();

      ppin3d->ClearExtents(&m_phitdrop->m_polydropanim.m_rcBounds, &m_phitdrop->m_polydropanim.m_znear, &m_phitdrop->m_polydropanim.m_zfar);
      ppin3d->ClearExtents(&pof->rc, NULL, NULL);
   }

   PinImage * const pinSide = m_ptable->GetImage(m_d.m_szSideImage);
   if (pinSide)
   {
      pinSide->EnsureColorKey();
      pd3dDevice->SetTexture(ePictureTexture, pinSide->m_pdsBufferColorKey);		

      if (pinSide->m_fTransparent)
      {				
         if (g_pvp->m_pdd.m_fHardwareAccel)
         {
            g_pplayer->m_pin3d.EnableAlphaTestReference(128);
         }
         else
            pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);

         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
      }
      else 
      {	
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
         pd3dDevice->SetRenderState(RenderDevice::DITHERENABLE, TRUE); 	
         g_pplayer->m_pin3d.EnableAlphaTestReference(g_pvp->m_pdd.m_fHardwareAccel ? 128 : 1);
         pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,   D3DBLEND_SRCALPHA);
         pd3dDevice->SetRenderState(RenderDevice::DESTBLEND,  D3DBLEND_INVSRCALPHA); 			
      }

      g_pplayer->m_pin3d.SetColorKeyEnabled(TRUE);
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
      g_pplayer->m_pin3d.SetTextureFilter( ePictureTexture, TEXTURE_MODE_TRILINEAR );

      ppin3d->SetMaterial( 1.0f, 1.0f, 1.0f, 1.0f );
   }
   else
   {
      ppin3d->SetMaterial( 1.0f, m_d.m_sidecolor );
   }

   if(!m_d.m_fEnableLighting)
      ppin3d->EnableLightMap(fFalse, -1);
   else
      ppin3d->EnableLightMap(fTrue, fDrop ? m_d.m_heightbottom : m_d.m_heighttop);

   // Render side
   int offset=0;
   for (int i=0;i<numVertices;i++, offset+=4)
   {
      if(!m_d.m_fEnableLighting)
      {
         if (!fDrop && m_d.m_fSideVisible) // Don't need to render walls if dropped, but we do need to extend the extrema
         {
            // Draw side.
            pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, sideVBuffer, offset, 4, (LPWORD)rgi0123, 4, 0);
            //pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_NOLIGHTING_VERTEX, &vertsNotLit[offset], 4, (LPWORD)rgi0123, 4, 0);
         }

         if (fMover)
         {
            // Only do two points - each segment has two new points
            ppin3d->ExpandExtents(&pof->rc, &vertsNotLit[offset], &m_phitdrop->m_polydropanim.m_znear, &m_phitdrop->m_polydropanim.m_zfar, 2, fFalse);
         }
      }
      else
      {
         if (!fDrop && m_d.m_fSideVisible) // Don't need to render walls if dropped, but we do need to extend the extrema
         {
            // Draw side.
            pd3dDevice->renderPrimitive( D3DPT_TRIANGLEFAN, sideVBuffer, offset, 4, (LPWORD)rgi0123, 4, 0);
            //pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, &verts[offset], 4, (LPWORD)rgi0123, 4, 0);
         }

         if (fMover)
         {
            // Only do two points - each segment has two new points
            ppin3d->ExpandExtents(&pof->rc, &verts[offset], &m_phitdrop->m_polydropanim.m_znear, &m_phitdrop->m_polydropanim.m_zfar, 2, fFalse);
         }
      }
   }

   if (m_d.m_fVisible)
   {
      if (pinSide)
      {
         if(!m_d.m_fEnableLighting)
            ppin3d->EnableLightMap(fFalse, -1);
         else
            ppin3d->EnableLightMap(fTrue, fDrop ? m_d.m_heightbottom : m_d.m_heighttop);

         pinSide->EnsureColorKey();
         pd3dDevice->SetTexture(ePictureTexture, pinSide->m_pdsBufferColorKey);

         if (pinSide->m_fTransparent)
         {
            pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
         }
         else // ppin3d->SetTexture(pin->m_pdsBuffer);
         {
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
            pd3dDevice->SetRenderState(RenderDevice::DITHERENABLE, TRUE); 	
            g_pplayer->m_pin3d.EnableAlphaTestReference(g_pvp->m_pdd.m_fHardwareAccel ? 128 : 1);
            pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
            pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA); 
         }

         g_pplayer->m_pin3d.SetColorKeyEnabled(TRUE);
         pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
         g_pplayer->m_pin3d.SetTextureFilter( ePictureTexture, TEXTURE_MODE_TRILINEAR );
      }
      else
         pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);

      PinImage * const pin = m_ptable->GetImage(m_d.m_szImage);
      if (pin)
      {
         pin->EnsureColorKey();
         pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);

         if (pin->m_fTransparent)
         {				
            pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
         }
         else 
         {
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
            pd3dDevice->SetRenderState(RenderDevice::DITHERENABLE, TRUE); 	
            g_pplayer->m_pin3d.EnableAlphaTestReference(g_pvp->m_pdd.m_fHardwareAccel ? 128 : 1);
            pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
            pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA); 
         }

         g_pplayer->m_pin3d.SetColorKeyEnabled(TRUE);
         pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
         g_pplayer->m_pin3d.SetTextureFilter( ePictureTexture, TEXTURE_MODE_TRILINEAR );

         ppin3d->SetMaterial( 1.0f, 1.0f, 1.0f, 1.0f );
      }
      else
      {
         ppin3d->SetTexture(NULL);
         ppin3d->SetMaterial( 1.0f, m_d.m_topcolor );
      }

      //!! combine drawcalls into one
      offset=0;
      const int rgi210[3]={2,1,0};
      for (int i=0;i<numPolys;i++, offset+=3)
      {
         if( !fDrop )
            pd3dDevice->renderPrimitive( D3DPT_TRIANGLELIST, topVBuffer[0], offset, 3, (LPWORD)rgi210,3,0);
         else
            pd3dDevice->renderPrimitive( D3DPT_TRIANGLELIST, topVBuffer[1], offset, 3, (LPWORD)rgi210,3,0);
      }
   }

   ppin3d->SetTexture(NULL);
   ppin3d->EnableLightMap(fFalse, -1);

   if (fMover)
   {
      // Create the color surface.
      pof->pdds = ppin3d->CreateOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
      pof->pdds->BltFast(0, 0, ppin3d->m_pddsBackBuffer, &pof->rc, DDBLTFAST_WAIT);

      // Create the z surface.
      pof->pddsZBuffer = ppin3d->CreateZBufferOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
      /*const HRESULT hr =*/ pof->pddsZBuffer->BltFast(0, 0, ppin3d->m_pddsZBuffer, &pof->rc, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);

      ppin3d->ExpandRectByRect(&m_phitdrop->m_polydropanim.m_rcBounds, &pof->rc);

      // reset the portion of the z-buffer that we changed
      ppin3d->m_pddsZBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
      // Reset color key in back buffer
      DDBLTFX ddbltfx;
      ddbltfx.dwSize = sizeof(DDBLTFX);
      ddbltfx.dwFillColor = 0;
      ppin3d->m_pddsBackBuffer->Blt(&pof->rc, NULL,&pof->rc, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
   }

   if(!m_d.m_fEnableLighting)
      pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);

   return pof;
}

void Surface::RenderMovers(const RenderDevice* pd3dDevice)
{
   RenderSlingshots((RenderDevice*)pd3dDevice);

   if (m_d.m_fDroppable)
   {
      // Render wall raised.
      ObjFrame * const pof = RenderWallsAtHeight((RenderDevice*)pd3dDevice, fTrue, fFalse);
      m_phitdrop->m_polydropanim.m_pobjframe[0] = pof;

      // Check if this wall is being 
      // used as a flipbook animation.
      if (m_d.m_fFlipbook)
      {
         // Don't render a dropped wall. 
         m_phitdrop->m_polydropanim.m_pobjframe[1] = NULL;
      }
      else
      {
         // Render wall dropped (smashed to a pancake at bottom height).
         ObjFrame * const pof2 = RenderWallsAtHeight((RenderDevice*)pd3dDevice, fTrue, fTrue); 
         m_phitdrop->m_polydropanim.m_pobjframe[1] = pof2;
      }
   }
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

         RECT rc;
         GetClientRect(m_ptable->m_hwnd, &rc);

         HitSur * const phs = new HitSur(NULL, m_ptable->m_zoom, m_ptable->m_offsetx, m_ptable->m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

         const Vertex2D v = phs->ScreenToSurface(x, y);
         delete phs;

         Vector<RenderVertex> vvertex;
         GetRgVertex(&vvertex);

         Vertex2D vOut;
         int iSeg;
         ClosestPointOnPolygon(vvertex, v, &vOut, &iSeg, true);

         // Go through vertices (including iSeg itself) counting control points until iSeg
         int icp = 0;
         for (int i=0;i<(iSeg+1);i++)
            if (vvertex.ElementAt(i)->fControlPoint)
               icp++;

         for (int i=0;i<vvertex.Size();i++)
            delete vvertex.ElementAt(i);

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

#ifdef VBA
   bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif

   bw.WriteBool(FID(HTEV), m_d.m_fHitEvent);
   bw.WriteBool(FID(DROP), m_d.m_fDroppable);
   bw.WriteBool(FID(FLIP), m_d.m_fFlipbook);
   bw.WriteBool(FID(CLDW), m_d.m_fCollidable);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteString(FID(SIMG), m_d.m_szSideImage);
   bw.WriteInt(FID(COLR), m_d.m_sidecolor);
   bw.WriteInt(FID(TCLR), m_d.m_topcolor);
   bw.WriteInt(FID(SCLR), m_d.m_slingshotColor);
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
   bw.WriteBool(FID(CSHD), m_d.m_fCastsShadow);
   bw.WriteBool(FID(VSBL), m_d.m_fVisible);
   bw.WriteBool(FID(SLGA), m_d.m_fSlingshotAnimation);
   bw.WriteBool(FID(SVBL), m_d.m_fSideVisible);
   bw.WriteBool(FID(ELIT), m_d.m_fEnableLighting);

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

      m_d.m_fInner = fTrue;
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
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_d.m_sidecolor);
      //if (!(m_d.m_sidecolor & MINBLACKMASK)) {m_d.m_sidecolor |= MINBLACK;}	// set minimum black
   }
   else if (id == FID(TCLR))
   {
      pbr->GetInt(&m_d.m_topcolor);
      //if (!(m_d.m_topcolor & MINBLACKMASK)) {m_d.m_topcolor |= MINBLACK;}	// set minimum black
   }
   else if (id == FID(SCLR))
   {
      pbr->GetInt(&m_d.m_slingshotColor);
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
   else if (id == FID(CSHD))
   {
      pbr->GetBool(&m_d.m_fCastsShadow);
   }
   else if (id == FID(ELIT))
   {
      pbr->GetBool(&m_d.m_fEnableLighting);
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

STDMETHODIMP Surface::get_SideColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_sidecolor;

   return S_OK;
}

STDMETHODIMP Surface::put_SideColor(OLE_COLOR newVal)
{
   STARTUNDO

      m_d.m_sidecolor = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Surface::get_SlingshotColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_slingshotColor;

   return S_OK;
}

STDMETHODIMP Surface::put_SlingshotColor(OLE_COLOR newVal)
{
   STARTUNDO

      m_d.m_slingshotColor = newVal;

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

STDMETHODIMP Surface::get_FaceColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_topcolor;

   return S_OK;
}

STDMETHODIMP Surface::put_FaceColor(OLE_COLOR newVal)
{
   STARTUNDO

      m_d.m_topcolor = newVal;

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

   pproppane = new PropertyPane(IDD_PROPWALL_PHYSICS, IDS_STATE);
   pvproppane->AddElement(pproppane);

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

STDMETHODIMP Surface::get_IsDropped(VARIANT_BOOL *pVal)
{
   if (!g_pplayer)
   {
      return E_FAIL;
   }

   *pVal = (VARIANT_BOOL)FTOVB(m_fIsDropped);

   return S_OK;
}

STDMETHODIMP Surface::put_IsDropped(VARIANT_BOOL newVal)
{
   if (!g_pplayer || !m_d.m_fDroppable)
   {
      return E_FAIL;
   }

   const BOOL fNewVal = VBTOF(newVal);

   if (m_fIsDropped != fNewVal)
   {
      m_fIsDropped = fNewVal;

      m_phitdrop->m_polydropanim.m_iframedesire = m_fIsDropped ? 1 : 0;

      for (int i=0;i<m_vhoDrop.Size();i++)
      {
         m_vhoDrop.ElementAt(i)->m_fEnabled = !m_fIsDropped && m_d.m_fCollidable; //disable hit on enities composing the object 
      }

#ifdef ULTRAPIN
      // Check if this surface has a user value.
      const int index = ((int) ((m_d.m_heighttop * 100.0f) - 111.0f + 0.25f));
      if ( (index >= 0) && (index < LIGHTHACK_MAX) )
      {
         // The same light is getting multiple updates per frame.
         // In the case of player lights, the light is on... then immediately turned off.
         // I don't know why this behavior happens; but it's causing problems.

         if ( (m_fIsDropped) &&
            (!g_pplayer->m_LightHackReadyForDrawLightHackFn[index]) )
         {
            // Set the value.
            g_pplayer->m_LightHackCurrentState[index] = m_fIsDropped;
            g_pplayer->m_LightHackReadyForDrawLightHackFn[index] = TRUE;
            g_pplayer->m_LastUpdateTime[index] = msec();
         }
      }
#endif
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
///////////////////////////////////////////////////////////

STDMETHODIMP Surface::get_CastsShadow(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fCastsShadow);

   return S_OK;
}

STDMETHODIMP Surface::put_CastsShadow(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fCastsShadow = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Surface::get_EnableLighting(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fEnableLighting);

   return S_OK;
}

STDMETHODIMP Surface::put_EnableLighting(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fEnableLighting = VBTOF(newVal);

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

      m_d.m_fCollidable = fNewVal;

   for (int i=0;i<m_vhoCollidable.Size();i++)
   {
      if (m_d.m_fDroppable) m_vhoCollidable.ElementAt(i)->m_fEnabled = fNewVal && !m_fIsDropped;
      else m_vhoCollidable.ElementAt(i)->m_fEnabled = fNewVal; //copy to hit checking on enities composing the object 
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
