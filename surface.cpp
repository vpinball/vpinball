#include "StdAfx.h"
//#include "forsyth.h"
#include "objloader.h"

Surface::Surface()
{
   m_menuid = IDR_SURFACEMENU;

   m_d.m_collidable = true;
   m_d.m_slingshotAnimation = true;
   m_d.m_inner = true;
   m_d.m_isBottomSolid = false;
   m_slingshotVBuffer = 0;
   m_VBuffer = 0;
   m_IBuffer = 0;
   m_propPhysics = NULL;
   m_d.m_overwritePhysics = true;
}

Surface::~Surface()
{
   FreeBuffers();
}

bool Surface::IsTransparent() const
{
   bool result = false;
   if (m_d.m_sideVisible)
   {
      result = m_ptable->GetMaterial(m_d.m_szSideMaterial)->m_bOpacityActive;
   }
   if (m_d.m_topBottomVisible)
   {
      result = result || m_ptable->GetMaterial(m_d.m_szTopMaterial)->m_bOpacityActive;
   }
   return result;
}

HRESULT Surface::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;
   m_isWall = true;

   const float width  = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Wall", "Width",  50.f) : 50.f;
   const float length = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Wall", "Length", 50.f) : 50.f;

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x - width, y - length, 0.f, false);
      m_vdpoint.push_back(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x - width, y + length, 0.f, false);
      m_vdpoint.push_back(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x + width, y + length, 0.f, false);
      m_vdpoint.push_back(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x + width, y - length, 0.f, false);
      m_vdpoint.push_back(pdp);
   }

   SetDefaults(fromMouseClick);

   return InitVBA(fTrue, 0, NULL);
}

void Surface::WriteRegDefaults()
{
   const char * strKeyName = m_isWall ? "DefaultProps\\Wall" : "DefaultProps\\Target";

   SaveValueBool(strKeyName, "TimerEnabled", m_d.m_tdr.m_TimerEnabled);
   SaveValueInt(strKeyName, "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SaveValueBool(strKeyName, "HitEvent", m_d.m_hitEvent);
   SaveValueFloat(strKeyName, "HitThreshold", m_d.m_threshold);
   SaveValueFloat(strKeyName, "SlingshotThreshold", m_d.m_slingshot_threshold);
   SaveValueString(strKeyName, "TopImage", m_d.m_szImage);
   SaveValueString(strKeyName, "SideImage", m_d.m_szSideImage);
   SaveValueBool(strKeyName, "Droppable", m_d.m_droppable);
   SaveValueBool(strKeyName, "Flipbook", m_d.m_flipbook);
   SaveValueBool(strKeyName, "IsBottomSolid", m_d.m_isBottomSolid);
   SaveValueFloat(strKeyName, "HeightBottom", m_d.m_heightbottom);
   SaveValueFloat(strKeyName, "HeightTop", m_d.m_heighttop);
   SaveValueBool(strKeyName, "DisplayTexture", m_d.m_displayTexture);
   SaveValueFloat(strKeyName, "SlingshotForce", m_d.m_slingshotforce);
   SaveValueBool(strKeyName, "SlingshotAnimation", m_d.m_slingshotAnimation);
   SaveValueFloat(strKeyName, "Elasticity", m_d.m_elasticity);
   SaveValueFloat(strKeyName, "ElasticityFallOff", m_d.m_elasticityFalloff);
   SaveValueFloat(strKeyName, "Friction", m_d.m_friction);
   SaveValueFloat(strKeyName, "Scatter", m_d.m_scatter);
   SaveValueBool(strKeyName, "Visible", m_d.m_topBottomVisible);
   SaveValueBool(strKeyName, "SideVisible", m_d.m_sideVisible);
   SaveValueBool(strKeyName, "Collidable", m_d.m_collidable);
   const int tmp = quantizeUnsigned<8>(clamp(m_d.m_disableLightingTop, 0.f, 1.f));
   SaveValueInt(strKeyName, "DisableLighting", (tmp == 1) ? 0 : tmp); // backwards compatible saving
   SaveValueFloat(strKeyName, "DisableLightingBelow", m_d.m_disableLightingBelow);
   SaveValueBool(strKeyName, "ReflectionEnabled", m_d.m_reflectionEnabled);
}

#if 0
HRESULT Surface::InitTarget(PinTable * const ptable, const float x, const float y, bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Target";

   m_ptable = ptable;
   m_isWall = false;

   const float width = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Width", 30.f) : 30.f;
   const float length = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Length", 6.f) : 6.f;

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x - width, y - length, 0.f, false);
      m_vdpoint.push_back(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x - width, y + length, 0.f, false);
      pdp->m_autoTexture = false;
      m_vdpoint.push_back(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x + width, y + length, 0.f, false);
      pdp->m_autoTexture = false;
      pdp->m_texturecoord = 1.0f;
      m_vdpoint.push_back(pdp);
   }
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, x + 30.0f, y - 6.0f, 0.f, false);
      m_vdpoint.push_back(pdp);
   }

   //SetDefaults();
   //Set seperate defaults for targets (SetDefaults sets the Wall defaults)

   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault(strKeyName, "TimerInterval", 100) : 100;
   m_d.m_hitEvent = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "HitEvent", true) : true;
   m_d.m_threshold = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HitThreshold", 2.0f) : 2.0f;
   m_d.m_slingshot_threshold = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "SlingshotThreshold", 0.0f) : 0.0f;
   m_d.m_inner = true; //!! Deprecated, do not use anymore

   HRESULT hr = LoadValueString(strKeyName, "TopImage", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage = "";

   hr = LoadValueString(strKeyName, "SideImage", m_d.m_szSideImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSideImage = "";

   m_d.m_droppable = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Droppable", false) : false;
   m_d.m_flipbook = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Flipbook", false) : false;
   m_d.m_isBottomSolid = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "IsBottomSolid", true) : false;

   m_d.m_heightbottom = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HeightBottom", 0.0f) : 0.0f;
   m_d.m_heighttop = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HeightTop", 50.0f) : 50.0f;

   m_d.m_displayTexture = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "DisplayTexture", false) : false;
   m_d.m_slingshotforce = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "SlingshotForce", 80.0f) : 80.0f;
   m_d.m_slingshotAnimation = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "SlingshotAnimation", true) : true;

   m_d.m_elasticity = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Elasticity", 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Scatter", 0) : 0;

   m_d.m_topBottomVisible = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_sideVisible = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "SideVisible", true) : true;
   m_d.m_collidable = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Collidable", true) : true;

   return InitVBA(fTrue, 0, NULL);
}
#endif

void Surface::SetDefaults(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Wall";

   HRESULT hr;

   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault(strKeyName, "TimerInterval", 100) : 100;
   m_d.m_hitEvent = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "HitEvent", false) : false;
   m_d.m_threshold = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HitThreshold", 2.0f) : 2.0f;
   m_d.m_slingshot_threshold = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "SlingshotThreshold", 0.0f) : 0.0f;
   m_d.m_inner = true; //!! Deprecated, do not use anymore

   char buf[MAXTOKEN] = { 0 };
   hr = LoadValueString(strKeyName, "TopImage", buf, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage.clear();
   else
      m_d.m_szImage = buf;

   hr = LoadValueString(strKeyName, "SideImage", buf, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSideImage.clear();
   else
      m_d.m_szSideImage = buf;

   m_d.m_droppable = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Droppable", false) : false;
   m_d.m_flipbook = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Flipbook", false) : false;
   m_d.m_isBottomSolid = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "IsBottomSolid", true) : false;

   m_d.m_heightbottom = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HeightBottom", 0.0f) : 0.0f;
   m_d.m_heighttop = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "HeightTop", 50.0f) : 50.0f;

   m_d.m_displayTexture = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "DisplayTexture", false) : false;
   m_d.m_slingshotforce = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "SlingshotForce", 80.0f) : 80.0f;
   m_d.m_slingshotAnimation = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "SlingshotAnimation", true) : true;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_topBottomVisible = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Visible", true) : true;
   m_d.m_sideVisible = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "SideVisible", true) : true;
   m_d.m_collidable = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "Collidable", true) : true;
   m_d.m_disableLightingTop = dequantizeUnsigned<8>(fromMouseClick ? LoadValueIntWithDefault(strKeyName, "DisableLighting", 0) : 0); // stored as uchar for backward compatibility
   m_d.m_disableLightingBelow = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "DisableLightingBelow", 0.f) : 0.f;
   m_d.m_reflectionEnabled = fromMouseClick ? LoadValueBoolWithDefault(strKeyName, "ReflectionEnabled", true) : true;
}


void Surface::UIRenderPass1(Sur * const psur)
{
   psur->SetFillColor(m_ptable->RenderSolid() ? m_vpinball->m_fillColor : -1);
   psur->SetObject(this);
   // Don't want border color to be over-ridden when selected - that will be drawn later
   psur->SetBorderColor(-1, false, 0);

   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   Texture *ppi;
   if (m_ptable->RenderSolid() && m_d.m_displayTexture && (ppi = m_ptable->GetImage(m_d.m_szImage)))
   {
      ppi->CreateGDIVersion();
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

void Surface::UIRenderPass2(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this); // For selected formatting
   psur->SetObject(NULL);

   {
      std::vector<RenderVertex> vvertex; //!! check/reuse from prerender
      GetRgVertex(vvertex);
      psur->Polygon(vvertex);
   }

   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   bool drawDragpoints = ((m_selectstate != eNotSelected) || m_vpinball->m_alwaysDrawDragPoints);

   if (!drawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
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

   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint[i];
      if (!(drawDragpoints || pdp->m_slingshot))
         continue;
      psur->SetFillColor(-1);
      psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(255, 0, 0), false, 0);

      if (drawDragpoints)
      {
         psur->SetObject(pdp);
         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }

      if (pdp->m_slingshot)
      {
         psur->SetObject(NULL);
         const CComObject<DragPoint> * const pdp2 = m_vdpoint[(i < m_vdpoint.size() - 1) ? (i + 1) : 0];
         psur->SetLineColor(RGB(0, 0, 0), false, 3);

         psur->Line(pdp->m_v.x, pdp->m_v.y, pdp2->m_v.x, pdp2->m_v.y);
      }
   }
}

void Surface::RenderBlueprint(Sur *psur, const bool solid)
{
   // Don't render dragpoints for blueprint
   if (solid)
      psur->SetFillColor(BLUEPRINT_SOLID_COLOR);
   else
      psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this); // For selected formatting
   psur->SetObject(NULL);

   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   psur->Polygon(vvertex);
}

void Surface::GetTimers(vector<HitTimer*> &pvht)
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

void Surface::GetHitShapes(vector<HitObject*> &pvho)
{
   CurvesToShapes(pvho);

   m_isDropped = false;
   m_disabled = false;
}

void Surface::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Surface/SurfaceHitGenerator.cs
//

void Surface::CurvesToShapes(vector<HitObject*> &pvho)
{
   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   const int count = (int)vvertex.size();
   Vertex3Ds * const rgv3Dt = new Vertex3Ds[count];
   Vertex3Ds * const rgv3Db = m_d.m_isBottomSolid ? new Vertex3Ds[count] : NULL;

   const float bottom = m_d.m_heightbottom + m_ptable->m_tableheight;
   const float top = m_d.m_heighttop + m_ptable->m_tableheight;

   for (int i = 0; i < count; ++i)
   {
      const RenderVertex * const pv1 = &vvertex[i];

      rgv3Dt[i].x = pv1->x;
      rgv3Dt[i].y = pv1->y;
      rgv3Dt[i].z = top;

      if (m_d.m_isBottomSolid)
      {
         rgv3Db[count - 1 - i].x = pv1->x;
         rgv3Db[count - 1 - i].y = pv1->y;
         rgv3Db[count - 1 - i].z = bottom;
      }

      const RenderVertex &pv2 = vvertex[(i + 1) % count];
      const RenderVertex &pv3 = vvertex[(i + 2) % count];

      AddLine(pvho, pv2, pv3);
   }

   Hit3DPoly * const ph3dpolyt = new Hit3DPoly(rgv3Dt, count);
   SetupHitObject(pvho, ph3dpolyt);

   if (m_d.m_isBottomSolid)
   {
      Hit3DPoly * const ph3dpolyb = new Hit3DPoly(rgv3Db, count);
      SetupHitObject(pvho, ph3dpolyb);
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Surface::SetupHitObject(vector<HitObject*> &pvho, HitObject * const obj)
{
   Material * const mat = m_ptable->GetMaterial(m_d.m_szPhysicsMaterial);
   if (!m_d.m_overwritePhysics)
   {
      obj->m_elasticity = mat->m_fElasticity;
      obj->m_elasticityFalloff = mat->m_fElasticityFalloff;
      obj->SetFriction(mat->m_fFriction);
      obj->m_scatter = ANGTORAD(mat->m_fScatterAngle);
   }
   else
   {
      obj->m_elasticity = m_d.m_elasticity;
      obj->m_elasticityFalloff = m_d.m_elasticityFalloff;
      obj->SetFriction(m_d.m_friction);
      obj->m_scatter = ANGTORAD(m_d.m_scatter);
   }

   obj->m_enabled = m_d.m_collidable;

   if (m_d.m_hitEvent)
   {
      obj->m_obj = (IFireEvents*)this;
      obj->m_fe = true;
      obj->m_threshold = m_d.m_threshold;
   }

   pvho.push_back(obj);
   m_vhoCollidable.push_back(obj);	//remember hit components of wall
   if (m_d.m_droppable)
      m_vhoDrop.push_back(obj);
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Surface/SurfaceHitGenerator.cs
//

void Surface::AddLine(vector<HitObject*> &pvho, const RenderVertex &pv1, const RenderVertex &pv2)
{
   const float bottom = m_d.m_heightbottom + m_ptable->m_tableheight;
   const float top = m_d.m_heighttop + m_ptable->m_tableheight;

   LineSeg *plineseg;
   if (!pv1.slingshot)
   {
      plineseg = new LineSeg(pv1, pv2, bottom, top);
   }
   else
   {
      LineSegSlingshot * const plinesling = new LineSegSlingshot(pv1, pv2, bottom, top);
      plineseg = (LineSeg *)plinesling;

      plinesling->m_force = m_d.m_slingshotforce;
      plinesling->m_psurface = this;

      m_vlinesling.push_back(plinesling);
   }

   SetupHitObject(pvho, plineseg);

   if (pv1.slingshot)  // slingshots always have hit events
   {
      plineseg->m_obj = (IFireEvents*)this;
      plineseg->m_fe = true;
      plineseg->m_threshold = m_d.m_threshold;
   }

   if (m_d.m_heightbottom != 0.f)
      // add lower edge as a line
      SetupHitObject(pvho, new HitLine3D(Vertex3Ds(pv1.x, pv1.y, bottom), Vertex3Ds(pv2.x, pv2.y, bottom)));

   // add upper edge as a line
   SetupHitObject(pvho, new HitLine3D(Vertex3Ds(pv1.x, pv1.y, top), Vertex3Ds(pv2.x, pv2.y, top)));

   // create vertical joint between the two line segments
   SetupHitObject(pvho, new HitLineZ(pv1, bottom, top));

   // add upper and lower end points of line
   if (m_d.m_heightbottom != 0.f)
      SetupHitObject(pvho, new HitPoint(Vertex3Ds(pv1.x, pv1.y, bottom)));
   SetupHitObject(pvho, new HitPoint(Vertex3Ds(pv1.x, pv1.y, top)));
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Surface::GetBoundingVertices(std::vector<Vertex3Ds>& pvvertex3D)
{
   // hardwired to table dimensions, but with bottom/top of surface, returns all 8 corners as this will be used for further transformations later-on
	for (int i = 0; i < 8; i++)
	{
		const Vertex3Ds pv(
			(i & 1) ? m_ptable->m_right : m_ptable->m_left,
			(i & 2) ? m_ptable->m_bottom : m_ptable->m_top,
			(i & 4) ? m_d.m_heighttop : m_d.m_heightbottom);

		pvvertex3D.push_back(pv);
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
   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint[i];

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }
}

void Surface::RenderDynamic()
{
   TRACE_FUNCTION();

   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   RenderSlingshots();

   if (m_d.m_droppable || m_isDynamic)
   {
      if (!m_isDropped)
      {
         // Render wall raised.
         RenderWallsAtHeight(false);
      }
      else    // is dropped
      {
         // if this wall is part of flipbook animation, do not render when dropped
         if (!m_d.m_flipbook)
         {
            // Render wall dropped (smashed to a pancake at bottom height).
            RenderWallsAtHeight(true);
         }
      }
   }
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Surface/SurfaceMeshGenerator.cs
//

void Surface::GenerateMesh(std::vector<Vertex3D_NoTex2> &topBuf, std::vector<Vertex3D_NoTex2> &sideBuf, std::vector<WORD> &topBottomIndices, std::vector<WORD> &sideIndices)
{
   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);
   float *rgtexcoord = NULL;

   Texture * const pinSide = m_ptable->GetImage(m_d.m_szSideImage);
   if (pinSide)
      GetTextureCoords(vvertex, &rgtexcoord);

   m_numVertices = (unsigned int)vvertex.size();
   Vertex2D * const rgnormal = new Vertex2D[m_numVertices];

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      const RenderVertex * const pv1 = &vvertex[i];
      const RenderVertex * const pv2 = &vvertex[(i < m_numVertices - 1) ? (i + 1) : 0];
      const float dx = pv1->x - pv2->x;
      const float dy = pv1->y - pv2->y;

      const float inv_len = 1.0f / sqrtf(dx*dx + dy*dy);

      rgnormal[i].x = dy*inv_len;
      rgnormal[i].y = dx*inv_len;
   }

   sideBuf.resize(m_numVertices * 4);
   memset(sideBuf.data(), 0, sizeof(Vertex3D_NoTex2)*m_numVertices * 4);
   Vertex3D_NoTex2 *verts = sideBuf.data();

   const float bottom = m_d.m_heightbottom + m_ptable->m_tableheight;
   const float top = m_d.m_heighttop + m_ptable->m_tableheight;

   int offset = 0;
   // Render side
   for (unsigned int i = 0; i < m_numVertices; i++, offset += 4)
   {
      const RenderVertex * const pv1 = &vvertex[i];
      const RenderVertex * const pv2 = &vvertex[(i < m_numVertices - 1) ? (i + 1) : 0];

      const int a = (i == 0) ? (m_numVertices - 1) : (i - 1);
      const int c = (i < m_numVertices - 1) ? (i + 1) : 0;

      Vertex2D vnormal[2];
      if (pv1->smooth)
      {
         vnormal[0].x = (rgnormal[a].x + rgnormal[i].x)*0.5f;
         vnormal[0].y = (rgnormal[a].y + rgnormal[i].y)*0.5f;
      }
      else
      {
         vnormal[0].x = rgnormal[i].x;
         vnormal[0].y = rgnormal[i].y;
      }

      if (pv2->smooth)
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
         verts[offset    ].x = pv1->x;   verts[offset    ].y = pv1->y;   verts[offset    ].z = bottom;
         verts[offset + 1].x = pv1->x;   verts[offset + 1].y = pv1->y;   verts[offset + 1].z = top;
         verts[offset + 2].x = pv2->x;   verts[offset + 2].y = pv2->y;   verts[offset + 2].z = top;
         verts[offset + 3].x = pv2->x;   verts[offset + 3].y = pv2->y;   verts[offset + 3].z = bottom;
         if (pinSide)
         {
            verts[offset].tu = rgtexcoord[i];
            verts[offset].tv = 1.0f;

            verts[offset + 1].tu = rgtexcoord[i];
            verts[offset + 1].tv = 0;

            verts[offset + 2].tu = rgtexcoord[c];
            verts[offset + 2].tv = 0;

            verts[offset + 3].tu = rgtexcoord[c];
            verts[offset + 3].tv = 1.0f;
         }

         verts[offset].nx = vnormal[0].x;
         verts[offset].ny = -vnormal[0].y;
         verts[offset].nz = 0;

         verts[offset + 1].nx = vnormal[0].x;
         verts[offset + 1].ny = -vnormal[0].y;
         verts[offset + 1].nz = 0;

         verts[offset + 2].nx = vnormal[1].x;
         verts[offset + 2].ny = -vnormal[1].y;
         verts[offset + 2].nz = 0;

         verts[offset + 3].nx = vnormal[1].x;
         verts[offset + 3].ny = -vnormal[1].y;
         verts[offset + 3].nz = 0;
      }
   }
   delete[] rgnormal;

   // prepare index buffer for sides
   {
      sideIndices.resize(m_numVertices * 6);
      int offset2 = 0;
      for (unsigned int i = 0; i < m_numVertices; i++, offset2 += 4)
      {
         sideIndices[i * 6    ] = offset2;
         sideIndices[i * 6 + 1] = offset2 + 1;
         sideIndices[i * 6 + 2] = offset2 + 2;
         sideIndices[i * 6 + 3] = offset2;
         sideIndices[i * 6 + 4] = offset2 + 2;
         sideIndices[i * 6 + 5] = offset2 + 3;
      }

      // not necessary to reorder
      /*WORD* const tmp = reorderForsyth(sideIndices, numVertices * 4);
      if (tmp != NULL)
      {
      memcpy(sideIndices.data(), tmp, sideIndices.size()*sizeof(WORD));
      delete[] tmp;
      }*/
   }

   // draw top
   SAFE_VECTOR_DELETE(rgtexcoord);
   //if (m_d.m_visible)      // Visible could still be set later if rendered dynamically
   {
      topBottomIndices.clear();

      {
      std::vector<unsigned int> vpoly(m_numVertices);
      for (unsigned int i = 0; i < m_numVertices; i++)
         vpoly[i] = i;

      PolygonToTriangles(vvertex, vpoly, topBottomIndices, false);
      }

      m_numPolys = (unsigned int)(topBottomIndices.size() / 3);
      if (m_numPolys == 0)
      {
         // no polys to render leave vertex buffer undefined 
         return;
      }

      const float heightNotDropped = m_d.m_heighttop;
      const float heightDropped = m_d.m_heightbottom + 0.1f;

      const float inv_tablewidth = 1.0f / (m_ptable->m_right - m_ptable->m_left);
      const float inv_tableheight = 1.0f / (m_ptable->m_bottom - m_ptable->m_top);

      topBuf.resize(m_numVertices * 3);
      Vertex3D_NoTex2 * const vertsTop[3] = { &topBuf[0], &topBuf[m_numVertices], &topBuf[m_numVertices*2] };

      for (unsigned int i = 0; i < m_numVertices; i++)
      {
         const RenderVertex * const pv0 = &vvertex[i];

         vertsTop[0][i].x = pv0->x;
         vertsTop[0][i].y = pv0->y;
         vertsTop[0][i].z = heightNotDropped + m_ptable->m_tableheight;
         vertsTop[0][i].tu = pv0->x * inv_tablewidth;
         vertsTop[0][i].tv = pv0->y * inv_tableheight;
         vertsTop[0][i].nx = 0;
         vertsTop[0][i].ny = 0;
         vertsTop[0][i].nz = 1.0f;

         vertsTop[1][i].x = pv0->x;
         vertsTop[1][i].y = pv0->y;
         vertsTop[1][i].z = heightDropped;
         vertsTop[1][i].tu = pv0->x * inv_tablewidth;
         vertsTop[1][i].tv = pv0->y * inv_tableheight;
         vertsTop[1][i].nx = 0;
         vertsTop[1][i].ny = 0;
         vertsTop[1][i].nz = 1.0f;

         vertsTop[2][i].x = pv0->x;
         vertsTop[2][i].y = pv0->y;
         vertsTop[2][i].z = m_d.m_heightbottom;
         vertsTop[2][i].tu = pv0->x * inv_tablewidth;
         vertsTop[2][i].tv = pv0->y * inv_tableheight;
         vertsTop[2][i].nx = 0;
         vertsTop[2][i].ny = 0;
         vertsTop[2][i].nz = -1.0f;
      }
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Surface::ExportMesh(FILE *f)
{
   const float oldBottomHeight = m_d.m_heightbottom;
   const float oldTopHeight = m_d.m_heighttop;

   m_d.m_heightbottom *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   m_d.m_heighttop *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   std::vector<Vertex3D_NoTex2> topBuf;
   std::vector<Vertex3D_NoTex2> sideBuf;
   std::vector<WORD> topBottomIndices;
   std::vector<WORD> sideIndices;
   GenerateMesh(topBuf, sideBuf, topBottomIndices, sideIndices);

   m_d.m_heightbottom = oldBottomHeight;
   m_d.m_heighttop = oldTopHeight;

   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), NULL, NULL);
   if (topBuf.size() > 0 && m_d.m_topBottomVisible && !m_d.m_sideVisible)
   {
      WaveFrontObj_WriteObjectName(f, name);
      WaveFrontObj_WriteVertexInfo(f, topBuf.data(), m_numVertices);
      const Texture * const tex = m_ptable->GetImage(m_d.m_szImage);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szTopMaterial);
      if (tex)
      {
         WaveFrontObj_WriteMaterial(m_d.m_szImage, tex->m_szPath, mat);
         WaveFrontObj_UseTexture(f, m_d.m_szImage);
      }
      else
      {
         WaveFrontObj_WriteMaterial("none", string(), mat);
         WaveFrontObj_UseTexture(f, "none");
      }
      WaveFrontObj_WriteFaceInfo(f, topBottomIndices);
      WaveFrontObj_UpdateFaceOffset(m_numVertices);
   }
   else if (topBuf.size() > 0 && m_d.m_topBottomVisible && m_d.m_sideVisible)
   {
      Vertex3D_NoTex2 * const tmp = new Vertex3D_NoTex2[m_numVertices * 5];
      memcpy(tmp, sideBuf.data(), sizeof(Vertex3D_NoTex2) * m_numVertices * 4);
      memcpy(&tmp[m_numVertices * 4], topBuf.data(), sizeof(Vertex3D_NoTex2)*m_numVertices);
      WaveFrontObj_WriteObjectName(f, name);
      WaveFrontObj_WriteVertexInfo(f, tmp, m_numVertices * 5);
      delete[] tmp;

      const Material * const mat = m_ptable->GetMaterial(m_d.m_szTopMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szTopMaterial, string(), mat);
      WaveFrontObj_UseTexture(f, m_d.m_szTopMaterial);
      WORD * const idx = new WORD[topBottomIndices.size() + sideIndices.size()];
      memcpy(idx, sideIndices.data(), sideIndices.size()*sizeof(WORD));
      for (size_t i = 0; i < topBottomIndices.size(); i++)
         idx[sideIndices.size() + i] = topBottomIndices[i] + m_numVertices * 4;
      WaveFrontObj_WriteFaceInfoList(f, idx, (unsigned int)(topBottomIndices.size() + sideIndices.size()));
      WaveFrontObj_UpdateFaceOffset(m_numVertices * 5);
      delete[] idx;
   }
   else if (!m_d.m_topBottomVisible && m_d.m_sideVisible)
   {
      WaveFrontObj_WriteObjectName(f, name);
      WaveFrontObj_WriteVertexInfo(f, sideBuf.data(), m_numVertices * 4);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szSideMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szSideMaterial, string(), mat);
      WaveFrontObj_UseTexture(f, m_d.m_szSideMaterial);
      WaveFrontObj_WriteFaceInfo(f, sideIndices);
      WaveFrontObj_UpdateFaceOffset(m_numVertices * 4);
   }
}

void Surface::PrepareWallsAtHeight()
{
   if (m_IBuffer)
      m_IBuffer->release();
   if (m_VBuffer)
      m_VBuffer->release();

   std::vector<Vertex3D_NoTex2> topBottomBuf;
   std::vector<Vertex3D_NoTex2> sideBuf;
   std::vector<WORD> topBottomIndices;
   std::vector<WORD> sideIndices;
   GenerateMesh(topBottomBuf, sideBuf, topBottomIndices, sideIndices);

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->CreateVertexBuffer(m_numVertices * 4 + ((topBottomBuf.size() > 0) ? m_numVertices * 3 : 0), 0, MY_D3DFVF_NOTEX2_VERTEX, &m_VBuffer);

   Vertex3D_NoTex2 *verts;
   m_VBuffer->lock(0, 0, (void**)&verts, VertexBuffer::WRITEONLY);
   memcpy(verts, sideBuf.data(), sizeof(Vertex3D_NoTex2)*m_numVertices * 4);

   if (topBottomBuf.size() > 0)
      //if (m_d.m_visible) // Visible could still be set later if rendered dynamically
      {
         memcpy(verts+m_numVertices * 4, topBottomBuf.data(), sizeof(Vertex3D_NoTex2)*m_numVertices * 3);
      }

   m_VBuffer->unlock();

   //

   pd3dDevice->CreateIndexBuffer((unsigned int)topBottomIndices.size() + (unsigned int)sideIndices.size(), 0, IndexBuffer::FMT_INDEX16, &m_IBuffer);

   WORD* buf;
   m_IBuffer->lock(0, 0, (void**)&buf, 0);
   memcpy(buf, sideIndices.data(), sideIndices.size() * sizeof(WORD));
   if (topBottomIndices.size() > 0)
      memcpy(buf + sideIndices.size(), topBottomIndices.data(), topBottomIndices.size() * sizeof(WORD));
   m_IBuffer->unlock();
}

static const WORD rgiSlingshot[24] = { 0, 4, 3, 0, 1, 4, 1, 2, 5, 1, 5, 4, 4, 8, 5, 4, 7, 8, 3, 7, 4, 3, 6, 7 };

static IndexBuffer* slingIBuffer = NULL;        // this is constant so we only have one global instance

void Surface::PrepareSlingshots()
{
   const float slingbottom = (m_d.m_heighttop - m_d.m_heightbottom) * 0.2f + m_d.m_heightbottom;
   const float slingtop = (m_d.m_heighttop - m_d.m_heightbottom) * 0.8f + m_d.m_heightbottom;

   Vertex3D_NoTex2* const rgv3D = new Vertex3D_NoTex2[m_vlinesling.size() * 9];

   unsigned int offset = 0;
   for (size_t i = 0; i < m_vlinesling.size(); i++, offset += 9)
   {
      LineSegSlingshot * const plinesling = m_vlinesling[i];
      plinesling->m_slingshotanim.m_animations = (m_d.m_slingshotAnimation != 0);

      rgv3D[offset].x = plinesling->v1.x;
      rgv3D[offset].y = plinesling->v1.y;
      rgv3D[offset].z = slingbottom + m_ptable->m_tableheight;

      rgv3D[offset + 1].x = (plinesling->v1.x + plinesling->v2.x)*0.5f + plinesling->normal.x*(m_d.m_slingshotforce * 0.25f); //40;//20;
      rgv3D[offset + 1].y = (plinesling->v1.y + plinesling->v2.y)*0.5f + plinesling->normal.y*(m_d.m_slingshotforce * 0.25f); //20;
      rgv3D[offset + 1].z = slingbottom + m_ptable->m_tableheight;

      rgv3D[offset + 2].x = plinesling->v2.x;
      rgv3D[offset + 2].y = plinesling->v2.y;
      rgv3D[offset + 2].z = slingbottom + m_ptable->m_tableheight;

      for (unsigned int l = 0; l < 3; l++)
      {
         rgv3D[l + offset + 3].x = rgv3D[l + offset].x;
         rgv3D[l + offset + 3].y = rgv3D[l + offset].y;
         rgv3D[l + offset + 3].z = slingtop + m_ptable->m_tableheight;
      }

      for (unsigned int l = 0; l < 3; l++)
      {
         rgv3D[l + offset + 6].x = rgv3D[l + offset].x - plinesling->normal.x*5.0f;
         rgv3D[l + offset + 6].y = rgv3D[l + offset].y - plinesling->normal.y*5.0f;
         rgv3D[l + offset + 6].z = slingtop + m_ptable->m_tableheight;
      }

      ComputeNormals(rgv3D + offset, 9, rgiSlingshot, 24);
   }

   if (m_slingshotVBuffer)
      m_slingshotVBuffer->release();

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->CreateVertexBuffer((unsigned int)m_vlinesling.size() * 9, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_slingshotVBuffer);

   Vertex3D_NoTex2 *buf;
   m_slingshotVBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   memcpy(buf, rgv3D, m_vlinesling.size() * 9 * sizeof(Vertex3D_NoTex2));
   m_slingshotVBuffer->unlock();

   delete[] rgv3D;

   if (!slingIBuffer)
      slingIBuffer = pd3dDevice->CreateAndFillIndexBuffer(24, rgiSlingshot);
}

void Surface::RenderSetup()
{
   const float oldBottomHeight = m_d.m_heightbottom;
   const float oldTopHeight = m_d.m_heighttop;

   m_d.m_heightbottom *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   m_d.m_heighttop *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   if (!m_vlinesling.empty())
      PrepareSlingshots();

   m_isDynamic = false;
   if (m_d.m_sideVisible)
   {
      if (m_ptable->GetMaterial(m_d.m_szSideMaterial)->m_bOpacityActive)
         m_isDynamic = true;
   }
   if (m_d.m_topBottomVisible)
   {
      if (m_ptable->GetMaterial(m_d.m_szTopMaterial)->m_bOpacityActive)
         m_isDynamic = true;
   }

   // create all vertices for dropped and non-dropped surface
   PrepareWallsAtHeight();
   m_d.m_heightbottom = oldBottomHeight;
   m_d.m_heighttop = oldTopHeight;
}

void Surface::FreeBuffers()
{
   if (m_slingshotVBuffer)
   {
      m_slingshotVBuffer->release();
      m_slingshotVBuffer = 0;
   }
   if (m_VBuffer)
   {
      m_VBuffer->release();
      m_VBuffer = 0;
   }
   if (m_IBuffer)
   {
      m_IBuffer->release();
      m_IBuffer = 0;
   }
   if (slingIBuffer)    // NB: global instance
   {
      slingIBuffer->release();
      slingIBuffer = 0;
   }
}

void Surface::RenderStatic()
{
   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   RenderSlingshots();
   if (!m_d.m_droppable && !m_isDynamic)
      RenderWallsAtHeight(false);
}

void Surface::RenderSlingshots()
{
   if (!m_d.m_sideVisible || (m_vlinesling.size() == 0))
      return;

   bool nothing_to_draw = true;
   for (size_t i = 0; i < m_vlinesling.size(); i++)
   {
      LineSegSlingshot * const plinesling = m_vlinesling[i];
      if (plinesling->m_slingshotanim.m_iframe || plinesling->m_doHitEvent)
      {
         nothing_to_draw = false;
         break;
      }
   }

   if (nothing_to_draw)
      return;

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szSlingShotMaterial);
   pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);

   pd3dDevice->basicShader->Begin(0);
   for (size_t i = 0; i < m_vlinesling.size(); i++)
   {
      LineSegSlingshot * const plinesling = m_vlinesling[i];
      if (!plinesling->m_slingshotanim.m_iframe && !plinesling->m_doHitEvent)
         continue;
      else if (plinesling->m_doHitEvent)
      {
          if (plinesling->m_EventTimeReset == 0)
             plinesling->m_EventTimeReset = g_pplayer->m_time_msec+100;
          else if (plinesling->m_EventTimeReset < g_pplayer->m_time_msec)
          {
              plinesling->m_doHitEvent = false;
              plinesling->m_EventTimeReset = 0;
          }
      }

      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_slingshotVBuffer, (DWORD)i * 9, 9, slingIBuffer, 0, 24);
   }
   pd3dDevice->basicShader->End();

   //pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
}

void Surface::RenderWallsAtHeight(const bool drop)
{
   if (m_ptable->m_reflectionEnabled && (/*m_d.m_heightbottom < 0.0f ||*/ m_d.m_heighttop < 0.0f))
      return;

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   if ((m_d.m_disableLightingTop != 0.f || m_d.m_disableLightingBelow != 0.f) && (m_d.m_sideVisible || m_d.m_topBottomVisible))
   {
      const vec4 tmp(m_d.m_disableLightingTop,m_d.m_disableLightingBelow, 0.f,0.f);
      pd3dDevice->basicShader->SetDisableLighting(tmp);
   }

   // render side
   if (m_d.m_sideVisible && !drop && (m_numVertices > 0)) // Don't need to render walls if dropped
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szSideMaterial);
      pd3dDevice->basicShader->SetMaterial(mat);

      pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);

      if (mat->m_bOpacityActive || !m_isDynamic)
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
      else
      {
         if (m_d.m_topBottomVisible && m_isDynamic)
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
         else
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
      }
      Texture * const pinSide = m_ptable->GetImage(m_d.m_szSideImage);
      if (pinSide)
      {
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
         pd3dDevice->basicShader->SetTexture("Texture0", pinSide, false);
         pd3dDevice->basicShader->SetAlphaTestValue(pinSide->m_alphaTestValue * (float)(1.0 / 255.0));

         //g_pplayer->m_pin3d.SetPrimaryTextureFilter( 0, TEXTURE_MODE_TRILINEAR );
      }
      else
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

      // combine drawcalls into one (hopefully faster)
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_VBuffer, 0, m_numVertices * 4, m_IBuffer, 0, m_numVertices * 6);
      pd3dDevice->basicShader->End();
   }

   // render top&bottom
   if (m_d.m_topBottomVisible && (m_numPolys > 0))
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szTopMaterial);
      pd3dDevice->basicShader->SetMaterial(mat);

      pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);

      if (mat->m_bOpacityActive || !m_isDynamic)
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
      else
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

      Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
      if (pin)
      {
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
         pd3dDevice->basicShader->SetTexture("Texture0", pin, false);
         pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));

         //g_pplayer->m_pin3d.SetTextureFilter( 0, TEXTURE_MODE_TRILINEAR );
      }
      else
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

      // Top
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_VBuffer, m_numVertices * 4 + (!drop ? 0 : m_numVertices), m_numVertices, m_IBuffer, m_numVertices * 6, m_numPolys * 3);
      pd3dDevice->basicShader->End();

      // Only render Bottom for Reflections
      if (m_ptable->m_reflectionEnabled)
      {
          if (mat->m_bOpacityActive || !m_isDynamic)
              pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);
          else
              pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CW);

          pd3dDevice->basicShader->Begin(0);
          pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_VBuffer, m_numVertices * 4 + m_numVertices * 2, m_numVertices, m_IBuffer, m_numVertices * 6, m_numPolys * 3);
          pd3dDevice->basicShader->End();
      }
   }

   // reset render states
   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!!  not necessary anymore
   //pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
   if ((m_d.m_disableLightingTop != 0.f || m_d.m_disableLightingBelow != 0.f) && (m_d.m_sideVisible || m_d.m_topBottomVisible))
   {
      const vec4 tmp(0.f,0.f, 0.f,0.f);
      pd3dDevice->basicShader->SetDisableLighting(tmp);
   }

}

void Surface::AddPoint(int x, int y, const bool smooth)
{
   STARTUNDO

   const Vertex2D v = m_ptable->TransformPoint(x, y);

   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   Vertex2D vOut;
   int iSeg;
   ClosestPointOnPolygon(vvertex, v, vOut, iSeg, true);

   // Go through vertices (including iSeg itself) counting control points until iSeg
   int icp = 0;
   for (int i = 0; i < (iSeg + 1); i++)
      if (vvertex[i].controlPoint)
         icp++;

   CComObject<DragPoint> *pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp)
   {
      pdp->AddRef();
      pdp->Init(this, vOut.x, vOut.y, 0.f, smooth);
      m_vdpoint.insert(m_vdpoint.begin() + icp, pdp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
   }

   STOPUNDO
}

void Surface::DoCommand(int icmd, int x, int y)
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
      AddPoint(x, y, false);
   }
   break;
   }
}

void Surface::FlipY(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Surface::FlipX(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Surface::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter, useElementCenter);
}

void Surface::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter, useElementCenter);
}

void Surface::Translate(const Vertex2D &pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

HRESULT Surface::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteBool(FID(HTEV), m_d.m_hitEvent);
   bw.WriteBool(FID(DROP), m_d.m_droppable);
   bw.WriteBool(FID(FLIP), m_d.m_flipbook);
   bw.WriteBool(FID(ISBS), m_d.m_isBottomSolid);
   bw.WriteBool(FID(CLDW), m_d.m_collidable);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteString(FID(SIMG), m_d.m_szSideImage);
   bw.WriteString(FID(SIMA), m_d.m_szSideMaterial);
   bw.WriteString(FID(TOMA), m_d.m_szTopMaterial);
   bw.WriteString(FID(SLMA), m_d.m_szSlingShotMaterial);
   bw.WriteFloat(FID(HTBT), m_d.m_heightbottom);
   bw.WriteFloat(FID(HTTP), m_d.m_heighttop);
   //bw.WriteBool(FID(INNR), m_d.m_inner); //!! Deprecated
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteBool(FID(DSPT), m_d.m_displayTexture);
   bw.WriteFloat(FID(SLGF), m_d.m_slingshotforce);
   bw.WriteFloat(FID(SLTH), m_d.m_slingshot_threshold);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(WFCT), m_d.m_friction);
   bw.WriteFloat(FID(WSCT), m_d.m_scatter);
   bw.WriteBool(FID(VSBL), m_d.m_topBottomVisible);
   bw.WriteBool(FID(SLGA), m_d.m_slingshotAnimation);
   bw.WriteBool(FID(SVBL), m_d.m_sideVisible);
   const int tmp = quantizeUnsigned<8>(clamp(m_d.m_disableLightingTop, 0.f, 1.f));
   bw.WriteInt(FID(DILI), (tmp == 1) ? 0 : tmp); // backwards compatible saving
   bw.WriteFloat(FID(DILB), m_d.m_disableLightingBelow);
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
   if (!m_d.m_inner) {
      const size_t cvertex = m_vdpoint.size();

      float miny = FLT_MAX;
      size_t minyindex = 0;

      // Find smallest y point - use it to connect with surrounding border
      for (size_t i = 0; i < cvertex; i++)
      {
         float y;
         m_vdpoint[i]->get_Y(&y);
         if (y < miny)
         {
            miny = y;
            minyindex = i;
         }
      }

      float tmpx;
      m_vdpoint[minyindex]->get_X(&tmpx);
      const float tmpy = miny /*- 1.0f*/; // put tiny gap in to avoid errors

      // swap list around
      std::reverse(m_vdpoint.begin(), m_vdpoint.end());

      CComObject<DragPoint> *pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_left, m_ptable->m_top, 0.f, false);
         m_vdpoint.insert(m_vdpoint.begin() + (cvertex - minyindex - 1), pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_right, m_ptable->m_top, 0.f, false);
         m_vdpoint.insert(m_vdpoint.begin() + (cvertex - minyindex - 1), pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_right + 1.0f, m_ptable->m_bottom, 0.f, false); //!!! +1 needed for whatever reason (triangulation screwed up)
         m_vdpoint.insert(m_vdpoint.begin() + (cvertex - minyindex - 1), pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_left, m_ptable->m_bottom, 0.f, false);
         m_vdpoint.insert(m_vdpoint.begin() + (cvertex - minyindex - 1), pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, m_ptable->m_left - 1.0f, m_ptable->m_top, 0.f, false); //!!! -1 needed for whatever reason (triangulation screwed up)
         m_vdpoint.insert(m_vdpoint.begin() + (cvertex - minyindex - 1), pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, tmpx, tmpy, 0.f, false);
         m_vdpoint.insert(m_vdpoint.begin() + (cvertex - minyindex - 1), pdp);
      }

      m_d.m_inner = true;
   }

   return S_OK;
}

bool Surface::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(HTEV): pbr->GetBool(&m_d.m_hitEvent); break;
   case FID(DROP): pbr->GetBool(&m_d.m_droppable); break;
   case FID(FLIP): pbr->GetBool(&m_d.m_flipbook); break;
   case FID(ISBS): pbr->GetBool(&m_d.m_isBottomSolid); break;
   case FID(CLDW): pbr->GetBool(&m_d.m_collidable); break;
   case FID(TMON): pbr->GetBool(&m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(&m_d.m_tdr.m_TimerInterval); break;
   case FID(THRS): pbr->GetFloat(&m_d.m_threshold); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(SIMG): pbr->GetString(m_d.m_szSideImage); break;
   case FID(SIMA): pbr->GetString(m_d.m_szSideMaterial); break;
   case FID(TOMA): pbr->GetString(m_d.m_szTopMaterial); break;
   case FID(MAPH): pbr->GetString(m_d.m_szPhysicsMaterial); break;
   case FID(SLMA): pbr->GetString(m_d.m_szSlingShotMaterial); break;
   case FID(HTBT): pbr->GetFloat(&m_d.m_heightbottom); break;
   case FID(HTTP): pbr->GetFloat(&m_d.m_heighttop); break;
   case FID(INNR): pbr->GetBool(&m_d.m_inner); break; //!! Deprecated, do not use anymore
   case FID(NAME): pbr->GetWideString(m_wzName); break;
   case FID(DSPT): pbr->GetBool(&m_d.m_displayTexture); break;
   case FID(SLGF): pbr->GetFloat(&m_d.m_slingshotforce); break;
   case FID(SLTH): pbr->GetFloat(&m_d.m_slingshot_threshold); break;
   case FID(ELAS): pbr->GetFloat(&m_d.m_elasticity); break;
   case FID(ELFO): pbr->GetFloat(&m_d.m_elasticityFalloff); break;
   case FID(WFCT): pbr->GetFloat(&m_d.m_friction); break;
   case FID(WSCT): pbr->GetFloat(&m_d.m_scatter); break;
   case FID(VSBL): pbr->GetBool(&m_d.m_topBottomVisible); break;
   case FID(OVPH): pbr->GetBool(&m_d.m_overwritePhysics); break;
   case FID(SLGA): pbr->GetBool(&m_d.m_slingshotAnimation); break;
   case FID(DILI):
   {
      int tmp;
      pbr->GetInt(&tmp);
      m_d.m_disableLightingTop = (tmp == 1) ? 1.f : dequantizeUnsigned<8>(tmp); // backwards compatible hacky loading!
      break;
   }
   case FID(DILB): pbr->GetFloat(&m_d.m_disableLightingBelow); break;
   case FID(SVBL): pbr->GetBool(&m_d.m_sideVisible); break;
   case FID(REEN): pbr->GetBool(&m_d.m_reflectionEnabled); break;
   default:
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
      break;
   }
   }
   return true;
}

HRESULT Surface::InitPostLoad()
{
   return S_OK;
}

void Surface::UpdateStatusBarInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, "TopHeight: %.03f | BottomHeight: %0.3f", m_vpinball->ConvertToUnit(m_d.m_heighttop), m_vpinball->ConvertToUnit(m_d.m_heightbottom));
   m_vpinball->SetStatusBarUnitInfo(tbuf, true);
}

STDMETHODIMP Surface::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_hitEvent);

   return S_OK;
}

STDMETHODIMP Surface::put_HasHitEvent(VARIANT_BOOL newVal)
{
   m_d.m_hitEvent = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP Surface::put_Threshold(float newVal)
{
   m_d.m_threshold = newVal;

   return S_OK;
}

STDMETHODIMP Surface::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }
   m_d.m_szImage = szImage;

   return S_OK;
}

STDMETHODIMP Surface::get_SideMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSideMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_SideMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
   m_d.m_szSideMaterial = buf;

   return S_OK;
}

STDMETHODIMP Surface::get_SlingshotMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSlingShotMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_SlingshotMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
   m_d.m_szSlingShotMaterial = buf;

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
   m_d.m_heightbottom = newVal;

   return S_OK;
}

STDMETHODIMP Surface::get_HeightTop(float *pVal)
{
   *pVal = m_d.m_heighttop;

   return S_OK;
}

STDMETHODIMP Surface::put_HeightTop(float newVal)
{
   m_d.m_heighttop = newVal;

   return S_OK;
}

STDMETHODIMP Surface::get_TopMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szTopMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_TopMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
   m_d.m_szTopMaterial = buf;

   return S_OK;
}

STDMETHODIMP Surface::get_PhysicsMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szPhysicsMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_PhysicsMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
   m_d.m_szPhysicsMaterial = buf;

   return S_OK;
}

STDMETHODIMP Surface::get_OverwritePhysics(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_overwritePhysics);

   return S_OK;
}

STDMETHODIMP Surface::put_OverwritePhysics(VARIANT_BOOL newVal)
{
   m_d.m_overwritePhysics = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::get_CanDrop(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_droppable);

   return S_OK;
}

STDMETHODIMP Surface::put_CanDrop(VARIANT_BOOL newVal)
{
   m_d.m_droppable = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::get_FlipbookAnimation(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_flipbook);

   return S_OK;
}

STDMETHODIMP Surface::put_FlipbookAnimation(VARIANT_BOOL newVal)
{
   m_d.m_flipbook = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::get_IsBottomSolid(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_isBottomSolid);

   return S_OK;
}

STDMETHODIMP Surface::put_IsBottomSolid(VARIANT_BOOL newVal)
{
   m_d.m_isBottomSolid = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::get_IsDropped(VARIANT_BOOL *pVal)
{
   if (!g_pplayer)
      return E_FAIL;

   *pVal = FTOVB(m_isDropped);

   return S_OK;
}

STDMETHODIMP Surface::put_IsDropped(VARIANT_BOOL newVal)
{
   if (!g_pplayer || !m_d.m_droppable)
      return E_FAIL;

   const bool val = VBTOb(newVal);

   if (m_isDropped != val)
   {
      m_isDropped = val;

      const bool b = !m_isDropped && m_d.m_collidable;
      if (m_vhoDrop.size() > 0 && m_vhoDrop[0]->m_enabled != b)
        for (size_t i = 0; i < m_vhoDrop.size(); i++) //!! costly
          m_vhoDrop[i]->m_enabled = b; //disable hit on entities composing the object 
   }

   return S_OK;
}

STDMETHODIMP Surface::get_DisplayTexture(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_displayTexture);

   return S_OK;
}

STDMETHODIMP Surface::put_DisplayTexture(VARIANT_BOOL newVal)
{
   m_d.m_displayTexture = VBTOb(newVal);

   return S_OK;
}

// Force value divided by 10 to make it look more like flipper strength value
STDMETHODIMP Surface::get_SlingshotStrength(float *pVal)
{
   *pVal = m_d.m_slingshotforce*(float)(1.0 / 10.0);

   return S_OK;
}

STDMETHODIMP Surface::put_SlingshotStrength(float newVal)
{
   m_d.m_slingshotforce = newVal*10.0f;

   return S_OK;
}

STDMETHODIMP Surface::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Surface::put_Elasticity(float newVal)
{
   m_d.m_elasticity = newVal;

   return S_OK;
}

STDMETHODIMP Surface::get_ElasticityFalloff(float* pVal)
{
   *pVal = m_d.m_elasticityFalloff;

   return S_OK;
}

STDMETHODIMP Surface::put_ElasticityFalloff(float newVal)
{
   m_d.m_elasticityFalloff = newVal;

   return S_OK;
}

STDMETHODIMP Surface::get_Friction(float *pVal)
{
   *pVal = m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Surface::put_Friction(float newVal)
{
   m_d.m_friction = clamp(newVal, 0.f, 1.f);

   return S_OK;
}

STDMETHODIMP Surface::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Surface::put_Scatter(float newVal)
{
   m_d.m_scatter = newVal;

   return S_OK;
}

STDMETHODIMP Surface::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_topBottomVisible);

   return S_OK;
}

STDMETHODIMP Surface::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_topBottomVisible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::get_SideImage(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSideImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Surface::put_SideImage(BSTR newVal)
{
   char szSideImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szSideImage, MAXTOKEN, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szSideImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }
   m_d.m_szSideImage = szSideImage;

   return S_OK;
}

STDMETHODIMP Surface::get_Disabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_disabled);

   return S_OK;
}

STDMETHODIMP Surface::put_Disabled(VARIANT_BOOL newVal)
{
   m_disabled = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::get_SideVisible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_sideVisible);

   return S_OK;
}

STDMETHODIMP Surface::put_SideVisible(VARIANT_BOOL newVal)
{
   m_d.m_sideVisible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_collidable);

   return S_OK;
}

STDMETHODIMP Surface::put_Collidable(VARIANT_BOOL newVal)
{
   const bool fNewVal = VBTOb(newVal);

   if (!g_pplayer)
       m_d.m_collidable = fNewVal;
   else
   {
       const bool b = m_d.m_droppable ? (fNewVal && !m_isDropped) : fNewVal;
       if (m_vhoCollidable.size() > 0 && m_vhoCollidable[0]->m_enabled != b)
           for (size_t i = 0; i < m_vhoCollidable.size(); i++) //!! costly
              m_vhoCollidable[i]->m_enabled = b; //copy to hit checking on entities composing the object 
   }

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
   m_d.m_slingshot_threshold = newVal;

   return S_OK;
}

STDMETHODIMP Surface::get_SlingshotAnimation(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_slingshotAnimation);

   return S_OK;
}

STDMETHODIMP Surface::put_SlingshotAnimation(VARIANT_BOOL newVal)
{
   m_d.m_slingshotAnimation = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::get_DisableLighting(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_disableLightingTop != 0.f);

   return S_OK;
}

STDMETHODIMP Surface::put_DisableLighting(VARIANT_BOOL newVal)
{
   m_d.m_disableLightingTop = VBTOb(newVal) ? 1.f : 0;

   return S_OK;
}

STDMETHODIMP Surface::get_BlendDisableLighting(float *pVal)
{
   *pVal = m_d.m_disableLightingTop;

   return S_OK;
}

STDMETHODIMP Surface::put_BlendDisableLighting(float newVal)
{
   m_d.m_disableLightingTop = newVal;

   return S_OK;
}

STDMETHODIMP Surface::get_BlendDisableLightingFromBelow(float *pVal)
{
   *pVal = m_d.m_disableLightingBelow;

   return S_OK;
}

STDMETHODIMP Surface::put_BlendDisableLightingFromBelow(float newVal)
{
   m_d.m_disableLightingBelow = newVal;

   return S_OK;
}

STDMETHODIMP Surface::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP Surface::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Surface::PlaySlingshotHit()
{
    for (size_t i = 0; i < m_vlinesling.size(); i++)
    {
        LineSegSlingshot * const plinesling = m_vlinesling[i];
        if (plinesling)
            plinesling->m_doHitEvent = true;
    }
    return S_OK;
}

void Surface::SetDefaultPhysics(bool fromMouseClick)
{
   static const char strKeyName[] = "DefaultProps\\Wall";
   m_d.m_elasticity = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Elasticity", 0.3f) : 0.3f;
   m_d.m_elasticityFalloff = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "ElasticityFallOff", 0.0f) : 0.0f;
   m_d.m_friction = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Friction", 0.3f) : 0.3f;
   m_d.m_scatter = fromMouseClick ? LoadValueFloatWithDefault(strKeyName, "Scatter", 0) : 0;
}
