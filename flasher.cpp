#include "StdAfx.h"

Flasher::Flasher()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_isVisible = true;
   m_d.m_depthBias = 0.0f;
   m_dynamicVertexBuffer = 0;
   m_dynamicIndexBuffer = 0;
   m_dynamicVertexBufferRegenerate = true;
   m_vertices = 0;
   m_propVisual = NULL;
   memset(m_d.m_szImageA, 0, sizeof(m_d.m_szImageA));
   memset(m_d.m_szImageB, 0, sizeof(m_d.m_szImageB));
   m_ptable = NULL;
   m_numVertices = 0;
   m_numPolys = 0;
   m_minx = FLT_MAX;
   m_maxx = -FLT_MAX;
   m_miny = FLT_MAX;
   m_maxy = -FLT_MAX;
}

Flasher::~Flasher()
{
   if (m_dynamicVertexBuffer) {
      m_dynamicVertexBuffer->release();
      m_dynamicVertexBuffer = 0;
   }
   if (m_dynamicIndexBuffer) {
      m_dynamicIndexBuffer->release();
      m_dynamicIndexBuffer = 0;
   }
   if (m_vertices)
   {
      delete[] m_vertices;
      m_vertices = 0;
   }
}

void Flasher::InitShape()
{
   if (m_vdpoint.empty())
   {
      // First time shape has been set to custom - set up some points
      const float x = m_d.m_vCenter.x;
      const float y = m_d.m_vCenter.y;
      const float size = 100.0f;

      CComObject<DragPoint> *pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x - size*0.5f, y - size*0.5f, 0.f, false);
         m_vdpoint.push_back(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x - size*0.5f, y + size*0.5f, 0.f, false);
         m_vdpoint.push_back(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x + size*0.5f, y + size*0.5f, 0.f, false);
         m_vdpoint.push_back(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x + size*0.5f, y - size*0.5f, 0.f, false);
         m_vdpoint.push_back(pdp);
      }
   }
}

HRESULT Flasher::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;
   m_d.m_isVisible = true;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   m_d.m_rotX = 0.0f;
   m_d.m_rotY = 0.0f;
   m_d.m_rotZ = 0.0f;
   SetDefaults(fromMouseClick);

   InitShape();

   InitVBA(fTrue, 0, NULL);

   return S_OK;
}

void Flasher::SetDefaults(bool fromMouseClick)
{
   m_d.m_height = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Flasher", "Height", 50.f) : 50.f;
   m_d.m_rotX = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Flasher", "RotX", 0.f) : 0.f;
   m_d.m_rotY = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Flasher", "RotY", 0.f) : 0.f;
   m_d.m_rotZ = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Flasher", "RotZ", 0.f) : 0.f;
   m_d.m_color = fromMouseClick ? LoadValueIntWithDefault("DefaultProps\\Flasher", "Color", RGB(50,200,50)) : RGB(50,200,50);
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Flasher", "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault("DefaultProps\\Flasher", "TimerInterval", 100) : 100;

   HRESULT hr = LoadValueString("DefaultProps\\Flasher", "ImageA", m_d.m_szImageA, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageA[0] = 0;

   hr = LoadValueString("DefaultProps\\Flasher", "ImageB", m_d.m_szImageB, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageB[0] = 0;

   m_d.m_alpha = fromMouseClick ? LoadValueIntWithDefault("DefaultProps\\Flasher", "Opacity", 100) : 100;

   m_d.m_intensity_scale = 1.0f;

   m_d.m_modulate_vs_add = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Flasher", "ModulateVsAdd", 0.9f) : 0.9f;
   m_d.m_filterAmount = fromMouseClick ? LoadValueIntWithDefault("DefaultProps\\Flasher", "FilterAmount", 100) : 100;
   m_d.m_isVisible = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Flasher", "Visible", true) : true;
   m_d.m_addBlend = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Flasher", "AddBlend", false) : false;
   m_d.m_isDMD = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Flasher", "DMD", false) : false;
   m_d.m_displayTexture = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Flasher", "DisplayTexture", false) : false;
   m_d.m_imagealignment = fromMouseClick ? (RampImageAlignment)LoadValueIntWithDefault("DefaultProps\\Flasher", "ImageMode", ImageModeWrap) : ImageModeWrap;
   m_d.m_filter = fromMouseClick ? (Filters)LoadValueIntWithDefault("DefaultProps\\Flasher", "Filter", Filter_Overlay) : Filter_Overlay;
}

void Flasher::WriteRegDefaults()
{
   SaveValueFloat("DefaultProps\\Flasher", "Height", m_d.m_height);
   SaveValueFloat("DefaultProps\\Flasher", "RotX", m_d.m_rotX);
   SaveValueFloat("DefaultProps\\Flasher", "RotY", m_d.m_rotY);
   SaveValueFloat("DefaultProps\\Flasher", "RotZ", m_d.m_rotZ);
   SaveValueInt("DefaultProps\\Flasher", "Color", m_d.m_color);
   SaveValueBool("DefaultProps\\Flasher", "TimerEnabled", m_d.m_tdr.m_TimerEnabled);
   SaveValueInt("DefaultProps\\Flasher", "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SaveValueString("DefaultProps\\Flasher", "ImageA", m_d.m_szImageA);
   SaveValueString("DefaultProps\\Flasher", "ImageB", m_d.m_szImageB);
   SaveValueInt("DefaultProps\\Flasher", "Alpha", m_d.m_alpha);
   SaveValueFloat("DefaultProps\\Flasher", "ModulateVsAdd", m_d.m_modulate_vs_add);
   SaveValueBool("DefaultProps\\Flasher", "Visible", m_d.m_isVisible);
   SaveValueBool("DefaultProps\\Flasher", "DisplayTexture", m_d.m_displayTexture);
   SaveValueBool("DefaultProps\\Flasher", "AddBlend", m_d.m_addBlend);
   SaveValueBool("DefaultProps\\Flasher", "DMD", m_d.m_isDMD);
   SaveValueInt("DefaultProps\\Flasher", "ImageMode", m_d.m_imagealignment);
   SaveValueInt("DefaultProps\\Flasher", "Filter", m_d.m_filter);
   SaveValueInt("DefaultProps\\Flasher", "FilterAmount", m_d.m_filterAmount);
}

void Flasher::UIRenderPass1(Sur * const psur)
{
   if (m_vdpoint.empty())
      InitShape();

   psur->SetFillColor(m_ptable->RenderSolid() ? m_vpinball->m_fillColor: -1);
   psur->SetObject(this);
   // Don't want border color to be over-ridden when selected - that will be drawn later
   psur->SetBorderColor(-1, false, 0);

   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);
   Texture *ppi;
   if (m_ptable->RenderSolid() && m_d.m_displayTexture && (ppi = m_ptable->GetImage(m_d.m_szImageA)))
   {
      ppi->CreateGDIVersion();
      if (m_d.m_imagealignment == ImageModeWrap)
      {
         float _minx = FLT_MAX;
         float _miny = FLT_MAX;
         float _maxx = -FLT_MAX;
         float _maxy = -FLT_MAX;
         for (size_t i = 0; i < vvertex.size(); i++)
         {
            if (vvertex[i].x < _minx) _minx = vvertex[i].x;
            if (vvertex[i].x > _maxx) _maxx = vvertex[i].x;
            if (vvertex[i].y < _miny) _miny = vvertex[i].y;
            if (vvertex[i].y > _maxy) _maxy = vvertex[i].y;
         }

         if (ppi->m_hbmGDIVersion)
            psur->PolygonImage(vvertex, ppi->m_hbmGDIVersion, _minx, _miny, _minx + (_maxx - _minx), _miny + (_maxy - _miny), ppi->m_width, ppi->m_height);
      }
      else
      {
         if (ppi->m_hbmGDIVersion)
            psur->PolygonImage(vvertex, ppi->m_hbmGDIVersion, m_ptable->m_left, m_ptable->m_top, m_ptable->m_right, m_ptable->m_bottom, ppi->m_width, ppi->m_height);
         else
         {
            // Do nothing for now to indicate to user that there is a problem
         }
      }
   }
   else
      psur->Polygon(vvertex);
}

void Flasher::UIRenderPass2(Sur * const psur)
{
   psur->SetFillColor(-1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this); // For selected formatting
   psur->SetObject(NULL);

   {
      std::vector<RenderVertex> vvertex; //!! check/reuse from UIRenderPass1
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

   if (drawDragpoints)
   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint[i];
      psur->SetFillColor(-1);
      psur->SetBorderColor(pdp->m_dragging ? RGB(0, 255, 0) : RGB(255, 0, 0), false, 0);
      psur->SetObject(pdp);

      psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
   }
}


void Flasher::RenderBlueprint(Sur *psur, const bool solid)
{
}

void Flasher::GetTimers(vector<HitTimer*> &pvht)
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

void Flasher::GetHitShapes(vector<HitObject*> &pvho)
{
}

void Flasher::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}


void Flasher::EndPlay()
{
   IEditable::EndPlay();

   if (m_dynamicVertexBuffer)
   {
      m_dynamicVertexBuffer->release();
      m_dynamicVertexBuffer = 0;
      m_dynamicVertexBufferRegenerate = true;
   }
   if (m_dynamicIndexBuffer)
   {
      m_dynamicIndexBuffer->release();
      m_dynamicIndexBuffer = 0;
   }
   if (m_vertices)
   {
      delete[] m_vertices;
      m_vertices = 0;
   }
}

void Flasher::UpdateMesh()
{
   const float height = (m_d.m_height + m_ptable->m_tableheight)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   const float movx = m_minx + (m_maxx - m_minx)*0.5f;
   const float movy = m_miny + (m_maxy - m_miny)*0.5f;

   Matrix3D tempMatrix, TMatrix;
   TMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
   tempMatrix.RotateYMatrix(ANGTORAD(m_d.m_rotY));
   tempMatrix.Multiply(TMatrix, TMatrix);
   tempMatrix.RotateXMatrix(ANGTORAD(m_d.m_rotX));
   tempMatrix.Multiply(TMatrix, TMatrix);

   tempMatrix.SetTranslation(m_d.m_vCenter.x,m_d.m_vCenter.y,height);
   tempMatrix.Multiply(TMatrix, tempMatrix);

   TMatrix.SetTranslation(
       -movx, //-m_d.m_vCenter.x,
       -movy, //-m_d.m_vCenter.y,
       0.f);
   tempMatrix.Multiply(TMatrix, tempMatrix);

   Vertex3D_TexelOnly *buf;
   m_dynamicVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      Vertex3D_TexelOnly vert = m_vertices[i];
      tempMatrix.MultiplyVector(vert, vert);
      buf[i] = vert;
   }

   m_dynamicVertexBuffer->unlock();
}

void Flasher::RenderSetup()
{
   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   m_numVertices = (unsigned int)vvertex.size();
   if (m_numVertices == 0)
   {
       // no polys to render leave vertex buffer undefined 
       m_numPolys = 0;
       return;
   }

   std::vector<WORD> vtri;
   
   {
   std::vector<unsigned int> vpoly(m_numVertices);
   for (unsigned int i = 0; i < m_numVertices; i++)
      vpoly[i] = i;

   PolygonToTriangles(vvertex, vpoly, vtri, false);
   }

   m_numPolys = (int)(vtri.size()/3);
   if (m_numPolys == 0)
   {
      // no polys to render leave vertex buffer undefined 
      return;
   }

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   if (m_dynamicIndexBuffer)
      m_dynamicIndexBuffer->release();
   pd3dDevice->CreateIndexBuffer(m_numPolys * 3, 0, IndexBuffer::FMT_INDEX16, &m_dynamicIndexBuffer);
   NumVideoBytes += (int)(m_numPolys * 3 * sizeof(WORD));

   WORD* bufi;
   m_dynamicIndexBuffer->lock(0, 0, (void**)&bufi, 0);
   memcpy(bufi, vtri.data(), vtri.size()*sizeof(WORD));
   m_dynamicIndexBuffer->unlock();

   if (m_dynamicVertexBuffer)
      m_dynamicVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(m_numVertices, USAGE_DYNAMIC, MY_D3DFVF_TEX, &m_dynamicVertexBuffer);
   NumVideoBytes += (int)(m_numVertices*sizeof(Vertex3D_TexelOnly));

   if (m_vertices)
      delete[] m_vertices;
   m_vertices = new Vertex3D_TexelOnly[m_numVertices];

   m_minx = FLT_MAX;
   m_miny = FLT_MAX;
   m_maxx = -FLT_MAX;
   m_maxy = -FLT_MAX;

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      const RenderVertex * const pv0 = &vvertex[i];

      m_vertices[i].x = pv0->x;
      m_vertices[i].y = pv0->y;
      m_vertices[i].z = 0;

      if (pv0->x > m_maxx) m_maxx = pv0->x;
      if (pv0->x < m_minx) m_minx = pv0->x;
      if (pv0->y > m_maxy) m_maxy = pv0->y;
      if (pv0->y < m_miny) m_miny = pv0->y;
   }

   const float inv_width = 1.0f / (m_maxx - m_minx);
   const float inv_height = 1.0f / (m_maxy - m_miny);
   const float inv_tablewidth = 1.0f / (m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f / (m_ptable->m_bottom - m_ptable->m_top);
   m_d.m_vCenter.x = m_minx + ((m_maxx - m_minx)*0.5f);
   m_d.m_vCenter.y = m_miny + ((m_maxy - m_miny)*0.5f);

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      if (m_d.m_imagealignment == ImageModeWrap)
      {
         m_vertices[i].tu = (m_vertices[i].x - m_minx)*inv_width;
         m_vertices[i].tv = (m_vertices[i].y - m_miny)*inv_height;
      }
      else
      {
         m_vertices[i].tu = m_vertices[i].x*inv_tablewidth;
         m_vertices[i].tv = m_vertices[i].y*inv_tableheight;
      }
   }
}

void Flasher::RenderStatic()
{
}

void Flasher::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(0, 0);
}

void Flasher::FlipY(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Flasher::FlipX(const Vertex2D& pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Flasher::Rotate(const float ang, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter, useElementCenter);
}

void Flasher::Scale(const float scalex, const float scaley, const Vertex2D& pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter, useElementCenter);
}

void Flasher::Translate(const Vertex2D &pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

void Flasher::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;
   for (size_t i = 0; i < m_vdpoint.size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint[i];

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }
}

void Flasher::DoCommand(int icmd, int x, int y)
{
   ISelect::DoCommand(icmd, x, y);

   switch (icmd)
   {
   case ID_WALLMENU_FLIP:
   {
      FlipPointY(GetPointCenter());
   }
   break;

   case ID_WALLMENU_MIRROR:
   {
      FlipPointX(GetPointCenter());
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
         pdp->Init(this, vOut.x, vOut.y, 0.f, false);
         m_vdpoint.insert(m_vdpoint.begin() + icp, pdp); // push the second point forward, and replace it with this one.  Should work when index2 wraps.
      }

      STOPUNDO
   }
   break;
   }
}

HRESULT Flasher::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteFloat(FID(FHEI), m_d.m_height);
   bw.WriteFloat(FID(FLAX), m_d.m_vCenter.x);
   bw.WriteFloat(FID(FLAY), m_d.m_vCenter.y);
   bw.WriteFloat(FID(FROX), m_d.m_rotX);
   bw.WriteFloat(FID(FROY), m_d.m_rotY);
   bw.WriteFloat(FID(FROZ), m_d.m_rotZ);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteString(FID(IMAG), m_d.m_szImageA);
   bw.WriteString(FID(IMAB), m_d.m_szImageB);
   bw.WriteInt(FID(FALP), m_d.m_alpha);
   bw.WriteFloat(FID(MOVA), m_d.m_modulate_vs_add);
   bw.WriteBool(FID(FVIS), m_d.m_isVisible);
   bw.WriteBool(FID(DSPT), m_d.m_displayTexture);
   bw.WriteBool(FID(ADDB), m_d.m_addBlend);
   bw.WriteBool(FID(IDMD), m_d.m_isDMD);
   bw.WriteFloat(FID(FLDB), m_d.m_depthBias);
   bw.WriteInt(FID(ALGN), m_d.m_imagealignment);
   bw.WriteInt(FID(FILT), m_d.m_filter);
   bw.WriteInt(FID(FIAM), m_d.m_filterAmount);
   ISelect::SaveData(pstm, hcrypthash);

   HRESULT hr;
   if (FAILED(hr = SavePointData(pstm, hcrypthash)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

void Flasher::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}


HRESULT Flasher::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

bool Flasher::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(FHEI): pbr->GetFloat(&m_d.m_height); break;
   case FID(FLAX): pbr->GetFloat(&m_d.m_vCenter.x); break;
   case FID(FLAY): pbr->GetFloat(&m_d.m_vCenter.y); break;
   case FID(FROX): pbr->GetFloat(&m_d.m_rotX); break;
   case FID(FROY): pbr->GetFloat(&m_d.m_rotY); break;
   case FID(FROZ): pbr->GetFloat(&m_d.m_rotZ); break;
   case FID(COLR): pbr->GetInt(&m_d.m_color); break;
   case FID(TMON): pbr->GetBool(&m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(&m_d.m_tdr.m_TimerInterval); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImageA); break;
   case FID(IMAB): pbr->GetString(m_d.m_szImageB); break;
   case FID(FALP):
   {
      int iTmp;
      pbr->GetInt(&iTmp);
      //if (iTmp>100) iTmp=100;
      if (iTmp < 0) iTmp = 0;
      m_d.m_alpha = iTmp;
      break;
   }
   case FID(MOVA): pbr->GetFloat(&m_d.m_modulate_vs_add); break;
   case FID(NAME): pbr->GetWideString(m_wzName); break;
   case FID(FVIS): pbr->GetBool(&m_d.m_isVisible); break;
   case FID(ADDB): pbr->GetBool(&m_d.m_addBlend); break;
   case FID(IDMD): pbr->GetBool(&m_d.m_isDMD); break;
   case FID(DSPT): pbr->GetBool(&m_d.m_displayTexture); break;
   case FID(FLDB): pbr->GetFloat(&m_d.m_depthBias); break;
   case FID(ALGN): pbr->GetInt(&m_d.m_imagealignment); break;
   case FID(FILT): pbr->GetInt(&m_d.m_filter); break;
   case FID(FIAM): pbr->GetInt(&m_d.m_filterAmount); break;
   default:
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
      break;
   }
   }
   return true;
}

HRESULT Flasher::InitPostLoad()
{
   return S_OK;
}

STDMETHODIMP Flasher::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IFlasher
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

   return S_FALSE;
}

STDMETHODIMP Flasher::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;
   m_vpinball->SetStatusBarUnitInfo("", true);

   return S_OK;
}

STDMETHODIMP Flasher::put_X(float newVal)
{
   if (m_d.m_vCenter.x != newVal)
   {
      m_d.m_vCenter.x = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Flasher::put_Y(float newVal)
{
   if (m_d.m_vCenter.y != newVal)
   {
      m_d.m_vCenter.y = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_RotX(float *pVal)
{
   *pVal = m_d.m_rotX;

   return S_OK;
}

STDMETHODIMP Flasher::put_RotX(float newVal)
{
   if (m_d.m_rotX != newVal)
   {
      m_d.m_rotX = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_RotY(float *pVal)
{
   *pVal = m_d.m_rotY;

   return S_OK;
}

STDMETHODIMP Flasher::put_RotY(float newVal)
{
   if (m_d.m_rotY != newVal)
   {
      m_d.m_rotY = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_RotZ(float *pVal)
{
   *pVal = m_d.m_rotZ;

   return S_OK;
}

STDMETHODIMP Flasher::put_RotZ(float newVal)
{
   if (m_d.m_rotZ != newVal)
   {
      m_d.m_rotZ = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Height(float *pVal)
{
   *pVal = m_d.m_height;

   return S_OK;
}

STDMETHODIMP Flasher::put_Height(float newVal)
{
   if (m_d.m_height != newVal)
   {
      m_d.m_height = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Flasher::put_Color(OLE_COLOR newVal)
{
   if (m_d.m_color != newVal)
   {
      m_d.m_color = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_ImageA(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImageA, -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_ImageA(BSTR newVal)
{
   char szImage[sizeof(m_d.m_szImageA)];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, sizeof(m_d.m_szImageA), NULL, NULL);

   if (strcmp(szImage, m_d.m_szImageA) != 0)
      strncpy_s(m_d.m_szImageA, szImage, sizeof(m_d.m_szImageA)-1);

   return S_OK;
}

STDMETHODIMP Flasher::get_ImageB(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImageB, -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_ImageB(BSTR newVal)
{
   char szImage[sizeof(m_d.m_szImageB)];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, sizeof(m_d.m_szImageB), NULL, NULL);

   if (strcmp(szImage, m_d.m_szImageB) != 0)
      strncpy_s(m_d.m_szImageB, szImage, sizeof(m_d.m_szImageB)-1);

   return S_OK;
}

STDMETHODIMP Flasher::get_Filter(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];

   switch (m_d.m_filter)
   {
   case Filter_Additive:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "Additive", -1, wz, MAXNAMEBUFFER);
      break;
   }
   case Filter_Multiply:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "Multiply", -1, wz, MAXNAMEBUFFER);
      break;
   }
   case Filter_Overlay:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "Overlay", -1, wz, MAXNAMEBUFFER);
      break;
   }
   case Filter_Screen:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "Screen", -1, wz, MAXNAMEBUFFER);
      break;
   }
   default:
      assert(!"Invalid Flasher Filter");
   case Filter_None:
   {
      MultiByteToWideCharNull(CP_ACP, 0, "None", -1, wz, MAXNAMEBUFFER);
      break;
   }
   }
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_Filter(BSTR newVal)
{
   char m_szFilter[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, m_szFilter, MAXNAMEBUFFER, NULL, NULL);

   if (strcmp(m_szFilter, "Additive") == 0 && m_d.m_filter != Filter_Additive)
   {
      m_d.m_filter = Filter_Additive;
      m_dynamicVertexBufferRegenerate = true;
   }
   else if (strcmp(m_szFilter, "Multiply") == 0 && m_d.m_filter != Filter_Multiply)
   {
      m_d.m_filter = Filter_Multiply;
      m_dynamicVertexBufferRegenerate = true;
   }
   else if (strcmp(m_szFilter, "Overlay") == 0 && m_d.m_filter != Filter_Overlay)
   {
      m_d.m_filter = Filter_Overlay;
      m_dynamicVertexBufferRegenerate = true;
   }
   else if (strcmp(m_szFilter, "Screen") == 0 && m_d.m_filter != Filter_Screen)
   {
      m_d.m_filter = Filter_Screen;
      m_dynamicVertexBufferRegenerate = true;
   }
   else if (strcmp(m_szFilter, "None") == 0 && m_d.m_filter != Filter_None)
   {
      m_d.m_filter = Filter_None;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Opacity(long *pVal)
{
   *pVal = m_d.m_alpha;
   return S_OK;
}

STDMETHODIMP Flasher::put_Opacity(long newVal)
{
   SetAlpha(newVal);
   return S_OK;
}

STDMETHODIMP Flasher::get_IntensityScale(float *pVal)
{
   *pVal = m_d.m_intensity_scale;
   return S_OK;
}

STDMETHODIMP Flasher::put_IntensityScale(float newVal)
{
   m_d.m_intensity_scale = newVal;
   return S_OK;
}

STDMETHODIMP Flasher::get_ModulateVsAdd(float *pVal)
{
   *pVal = m_d.m_modulate_vs_add;
   return S_OK;
}

STDMETHODIMP Flasher::put_ModulateVsAdd(float newVal)
{
   m_d.m_modulate_vs_add = newVal;
   return S_OK;
}

STDMETHODIMP Flasher::get_Amount(long *pVal)
{
   *pVal = m_d.m_filterAmount;
   return S_OK;
}

STDMETHODIMP Flasher::put_Amount(long newVal)
{
   SetFilterAmount(newVal);
   return S_OK;
}

STDMETHODIMP Flasher::get_Visible(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = FTOVB(m_d.m_isVisible);

   return S_OK;
}

STDMETHODIMP Flasher::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_isVisible = VBTOb(newVal); // set visibility
   return S_OK;
}

STDMETHODIMP Flasher::get_DisplayTexture(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = FTOVB(m_d.m_displayTexture);

   return S_OK;
}

STDMETHODIMP Flasher::put_DisplayTexture(VARIANT_BOOL newVal)
{
   m_d.m_displayTexture = VBTOb(newVal); // set visibility
   return S_OK;
}

STDMETHODIMP Flasher::get_AddBlend(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = FTOVB(m_d.m_addBlend);

   return S_OK;
}

STDMETHODIMP Flasher::put_AddBlend(VARIANT_BOOL newVal)
{
   m_d.m_addBlend = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Flasher::get_DMD(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = FTOVB(m_d.m_isDMD);

   return S_OK;
}

STDMETHODIMP Flasher::put_DMD(VARIANT_BOOL newVal)
{
   m_d.m_isDMD = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Flasher::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Flasher::put_DepthBias(float newVal)
{
   if (m_d.m_depthBias != newVal)
   {
      m_d.m_depthBias = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_ImageAlignment(RampImageAlignment *pVal)
{
   *pVal = m_d.m_imagealignment;

   return S_OK;
}

STDMETHODIMP Flasher::put_ImageAlignment(RampImageAlignment newVal)
{
   if (m_d.m_imagealignment != newVal)
   {
      m_d.m_imagealignment = newVal;
      m_dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

void Flasher::RenderDynamic()
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   TRACE_FUNCTION();

   // Don't render if invisible (or DMD connection not set)
   if (!m_d.m_isVisible || m_dynamicVertexBuffer == NULL || m_ptable->m_reflectionEnabled || (m_d.m_isDMD && !g_pplayer->m_texdmd))
      return;

   const vec4 color = convertColor(m_d.m_color, (float)m_d.m_alpha*m_d.m_intensity_scale / 100.0f);
   if (color.w == 0.f)
      return;

   if (color.x == 0.f && color.y == 0.f && color.z == 0.f)
      return;

   if (m_d.m_isDMD && (g_pplayer->m_dmdstate == 0)) // don't draw any DMD, but this case should not happen in the first place
      return;

   const bool alphadmd = (m_d.m_modulate_vs_add < 1.f);

   if (m_d.m_isDMD &&
       (((g_pplayer->m_dmdstate == 1) && alphadmd) || // render alpha DMD
        ((g_pplayer->m_dmdstate == 2) && !alphadmd))) // render normal DMD
   {
       if (m_dynamicVertexBufferRegenerate)
       {
         UpdateMesh();
         m_dynamicVertexBufferRegenerate = false;
       }

       pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
       pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);

       pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
       if ((g_pplayer->m_dmdstate == 1) && alphadmd)
          g_pplayer->m_pin3d.EnableAlphaBlend(m_d.m_addBlend);
       else
          g_pplayer->m_pin3d.DisableAlphaBlend();

       /*const unsigned int alphamode = 1; //!! make configurable?
       // add
       if (alphamode == 1) {
         g_pplayer->m_pin3d.EnableAlphaBlend(true);
       // max
       } else if (alphamode == 2) {
         pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_TRUE);
         pd3dDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_MAX);
         pd3dDevice->SetRenderState(RenderDevice::SRCBLEND, RenderDevice::SRC_ALPHA);
         pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::INVSRC_COLOR);
       //subtract
       } else if (alphamode == 3) {
         pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_TRUE);
         pd3dDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_SUBTRACT);
         pd3dDevice->SetRenderState(RenderDevice::SRCBLEND, RenderDevice::ZERO);
         pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::INVSRC_COLOR);
       // normal
       } else if (alphamode == 4) {
         pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_TRUE);
         pd3dDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD);
         pd3dDevice->SetRenderState(RenderDevice::SRCBLEND, RenderDevice::SRC_ALPHA);
         pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, RenderDevice::INVSRC_ALPHA);
       }

       const int alphatest = 0; //!!
       if (alphatest)
         g_pplayer->m_pin3d.EnableAlphaTestReference(0x80);*/

       //const float width = g_pplayer->m_pin3d.m_useAA ? 2.0f*(float)m_width : (float)m_width; //!! AA ?? -> should just work
       pd3dDevice->DMDShader->SetTechnique("basic_DMD_world"); //!! DMD_UPSCALE ?? -> should just work

       pd3dDevice->DMDShader->SetVector("vColor_Intensity", &color);

#ifdef DMD_UPSCALE
       const vec4 r((float)(g_pplayer->m_dmd.x*3), (float)(g_pplayer->m_dmd.y*3), m_d.m_modulate_vs_add, (float)(g_pplayer->m_overall_frames%2048)); //(float)(0.5 / m_width), (float)(0.5 / m_height));
#else
       const vec4 r((float)g_pplayer->m_dmd.x, (float)g_pplayer->m_dmd.y, m_d.m_modulate_vs_add, (float)(g_pplayer->m_overall_frames%2048)); //(float)(0.5 / m_width), (float)(0.5 / m_height));
#endif
       pd3dDevice->DMDShader->SetVector("vRes_Alpha_time", &r);

       pd3dDevice->DMDShader->SetTexture("Texture0", g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_texMan.LoadTexture(g_pplayer->m_texdmd, false));

       pd3dDevice->DMDShader->Begin(0);
       pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_TEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numPolys * 3);
       pd3dDevice->DMDShader->End();
   }
   else if (g_pplayer->m_dmdstate == 0)
   {
       if (m_dynamicVertexBufferRegenerate)
       {
          UpdateMesh();
          m_dynamicVertexBufferRegenerate = false;
       }

       pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
       pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_NONE);

       Texture * const pinA = m_ptable->GetImage(m_d.m_szImageA);
       Texture * const pinB = m_ptable->GetImage(m_d.m_szImageB);

       bool hdrTex0;
       if (pinA && !pinB)
           hdrTex0 = pinA->IsHDR();
       else if (!pinA && pinB)
           hdrTex0 = pinB->IsHDR();
       else if (pinA && pinB)
           hdrTex0 = pinA->IsHDR();
       else
           hdrTex0 = false;

       const vec4 ab((float)m_d.m_filterAmount / 100.0f, min(max(m_d.m_modulate_vs_add, 0.00001f), 0.9999f), // avoid 0, as it disables the blend and avoid 1 as it looks not good with day->night changes
           hdrTex0 ? 1.f : 0.f, (pinA && pinB && pinB->IsHDR()) ? 1.f : 0.f);
       pd3dDevice->flasherShader->SetVector("amount__blend_modulate_vs_add__hdrTexture01", &ab);

       pd3dDevice->flasherShader->SetFlasherColorAlpha(color);

       vec4 flasherData(-1.f, -1.f, (float)m_d.m_filter, m_d.m_addBlend ? 1.f : 0.f);
       float flasherMode;
       pd3dDevice->flasherShader->SetTechnique("basic_noLight");

       if (pinA && !pinB)
       {
           flasherMode = 0.f;
           pd3dDevice->flasherShader->SetTexture("Texture0", pinA, false);

           if (!m_d.m_addBlend)
               flasherData.x = pinA->m_alphaTestValue * (float)(1.0 / 255.0);

           //ppin3d->SetPrimaryTextureFilter( 0, TEXTURE_MODE_TRILINEAR );
       }
       else if (!pinA && pinB)
       {
           flasherMode = 0.f;
           pd3dDevice->flasherShader->SetTexture("Texture0", pinB, false);

           if (!m_d.m_addBlend)
               flasherData.x = pinB->m_alphaTestValue * (float)(1.0 / 255.0);

           //ppin3d->SetPrimaryTextureFilter( 0, TEXTURE_MODE_TRILINEAR );
       }
       else if (pinA && pinB)
       {
           flasherMode = 1.f;
           pd3dDevice->flasherShader->SetTexture("Texture0", pinA, false);
           pd3dDevice->flasherShader->SetTexture("Texture1", pinB, false);

           if (!m_d.m_addBlend)
           {
               flasherData.x = pinA->m_alphaTestValue * (float)(1.0 / 255.0);
               flasherData.y = pinB->m_alphaTestValue * (float)(1.0 / 255.0);
           }

           //ppin3d->SetPrimaryTextureFilter( 0, TEXTURE_MODE_TRILINEAR );
       }
       else
           flasherMode = 2.f;

       pd3dDevice->flasherShader->SetFlasherData(flasherData, flasherMode);

       pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_FALSE);
       g_pplayer->m_pin3d.EnableAlphaBlend(m_d.m_addBlend, false, false);
       pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, m_d.m_addBlend ? RenderDevice::INVSRC_COLOR : RenderDevice::INVSRC_ALPHA);
       pd3dDevice->SetRenderState(RenderDevice::BLENDOP, m_d.m_addBlend ? RenderDevice::BLENDOP_REVSUBTRACT : RenderDevice::BLENDOP_ADD);

       pd3dDevice->flasherShader->Begin(0);
       pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_TEX, m_dynamicVertexBuffer, 0, m_numVertices, m_dynamicIndexBuffer, 0, m_numPolys * 3);
       pd3dDevice->flasherShader->End();
   }

   //pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
   //pd3dDevice->SetRenderState(RenderDevice::BLENDOP, RenderDevice::BLENDOP_ADD); //!! not necessary anymore
   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
}
