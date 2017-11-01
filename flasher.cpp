#include "StdAfx.h"

Flasher::Flasher()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_IsVisible = true;
   m_d.m_depthBias = 0.0f;
   dynamicVertexBuffer = 0;
   dynamicIndexBuffer = 0;
   dynamicVertexBufferRegenerate = true;
   vertices = 0;
   m_propVisual = NULL;
   memset(m_d.m_szImageA, 0, MAXTOKEN);
   memset(m_d.m_szImageB, 0, MAXTOKEN);
   m_ptable = NULL;
   numVertices = 0;
   numPolys = 0;
   minx = FLT_MAX;
   maxx = FLT_MIN;
   miny = FLT_MAX;
   maxy = FLT_MIN;
}

Flasher::~Flasher()
{
   if (dynamicVertexBuffer) {
      dynamicVertexBuffer->release();
      dynamicVertexBuffer = 0;
   }
   if (dynamicIndexBuffer) {
      dynamicIndexBuffer->release();
      dynamicIndexBuffer = 0;
   }
}

void Flasher::InitShape()
{
   if (m_vdpoint.Size() == 0)
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
         pdp->Init(this, x - size*0.5f, y - size*0.5f);
         pdp->m_fSmooth = false;
         m_vdpoint.AddElement(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x - size*0.5f, y + size*0.5f);
         pdp->m_fSmooth = false;
         m_vdpoint.AddElement(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x + size*0.5f, y + size*0.5f);
         pdp->m_fSmooth = false;
         m_vdpoint.AddElement(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x + size*0.5f, y - size*0.5f);
         pdp->m_fSmooth = false;
         m_vdpoint.AddElement(pdp);
      }
   }
}

HRESULT Flasher::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;
   m_d.m_IsVisible = true;

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
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Flasher", "Height", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_height = fTmp;
   else
      m_d.m_height = 50.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flasher", "RotX", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotX = fTmp;
   else
      m_d.m_rotX = 0.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flasher", "RotY", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotY = fTmp;
   else
      m_d.m_rotY = 0.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flasher", "RotZ", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotZ = fTmp;
   else
      m_d.m_rotZ = 0.0f;

   hr = GetRegInt("DefaultProps\\Flasher", "Color", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_color = iTmp;
   else
      m_d.m_color = RGB(50, 200, 50);

   hr = GetRegInt("DefaultProps\\Flasher", "TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\Flasher", "TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegString("DefaultProps\\Flasher", "ImageA", m_d.m_szImageA, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageA[0] = 0;

   hr = GetRegString("DefaultProps\\Flasher", "ImageB", m_d.m_szImageB, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageB[0] = 0;

   hr = GetRegInt("DefaultProps\\Flasher", "Opacity", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fAlpha = iTmp;
   else
      m_d.m_fAlpha = 100;

   m_d.m_intensity_scale = 1.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flasher", "ModulateVsAdd", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_modulate_vs_add = fTmp;
   else
      m_d.m_modulate_vs_add = 0.9f;

   hr = GetRegInt("DefaultProps\\Flasher", "FilterAmount", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fFilterAmount = iTmp;
   else
      m_d.m_fFilterAmount = 100;

   hr = GetRegInt("DefaultProps\\Flasher", "Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_IsVisible = iTmp == 0 ? false : true;
   else
      m_d.m_IsVisible = true;

   hr = GetRegInt("DefaultProps\\Flasher", "AddBlend", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fAddBlend = iTmp == 0 ? false : true;
   else
      m_d.m_fAddBlend = false;

   hr = GetRegInt("DefaultProps\\Flasher", "DMD", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
	   m_d.m_IsDMD = iTmp == 0 ? false : true;
   else
	   m_d.m_IsDMD = false;

   hr = GetRegInt("DefaultProps\\Flasher", "DisplayTexture", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fDisplayTexture = (iTmp == 0) ? false : true;
   else
      m_d.m_fDisplayTexture = false;

   m_d.m_imagealignment = fromMouseClick ? (RampImageAlignment)GetRegIntWithDefault("DefaultProps\\Flasher", "ImageMode", ImageModeWrap) : ImageModeWrap;
   m_d.m_filter = fromMouseClick ? (Filters)GetRegIntWithDefault("DefaultProps\\Flasher", "Filter", Filter_Overlay) : Filter_Overlay;
}

void Flasher::WriteRegDefaults()
{
   SetRegValueFloat("DefaultProps\\Flasher", "Height", m_d.m_height);
   SetRegValueFloat("DefaultProps\\Flasher", "RotX", m_d.m_rotX);
   SetRegValueFloat("DefaultProps\\Flasher", "RotY", m_d.m_rotY);
   SetRegValueFloat("DefaultProps\\Flasher", "RotZ", m_d.m_rotZ);
   SetRegValue("DefaultProps\\Flasher", "Color", REG_DWORD, &m_d.m_color, 4);
   SetRegValueBool("DefaultProps\\Flasher", "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValueInt("DefaultProps\\Flasher", "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SetRegValue("DefaultProps\\Flasher", "ImageA", REG_SZ, &m_d.m_szImageA, lstrlen(m_d.m_szImageA));
   SetRegValue("DefaultProps\\Flasher", "ImageB", REG_SZ, &m_d.m_szImageB, lstrlen(m_d.m_szImageB));
   SetRegValueInt("DefaultProps\\Flasher", "Alpha", m_d.m_fAlpha);
   SetRegValueFloat("DefaultProps\\Flasher", "ModulateVsAdd", m_d.m_modulate_vs_add);
   SetRegValueBool("DefaultProps\\Flasher", "Visible", m_d.m_IsVisible);
   SetRegValueBool("DefaultProps\\Flasher", "DisplayTexture", m_d.m_fDisplayTexture);
   SetRegValueBool("DefaultProps\\Flasher", "AddBlend", m_d.m_fAddBlend);
   SetRegValueBool("DefaultProps\\Flasher", "DMD", m_d.m_IsDMD);
   SetRegValue("DefaultProps\\Flasher", "ImageMode", REG_DWORD, &m_d.m_imagealignment, 4);
   SetRegValue("DefaultProps\\Flasher", "Filter", REG_DWORD, &m_d.m_filter, 4);
   SetRegValueInt("DefaultProps\\Flasher", "FilterAmount", m_d.m_fFilterAmount);
}

void Flasher::PreRender(Sur * const psur)
{
   if (m_vdpoint.Size() == 0)
      InitShape();

   psur->SetFillColor(m_ptable->RenderSolid() ? g_pvp->m_fillColor: -1);
   psur->SetObject(this);
   // Don't want border color to be over-ridden when selected - that will be drawn later
   psur->SetBorderColor(-1, false, 0);

   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);
   Texture *ppi;
   if (m_ptable->RenderSolid() && m_d.m_fDisplayTexture && (ppi = m_ptable->GetImage(m_d.m_szImageA)))
   {
      ppi->EnsureHBitmap();
      if (m_d.m_imagealignment == ImageModeWrap)
      {
         float _minx = FLT_MAX;
         float _miny = FLT_MAX;
         float _maxx = -FLT_MAX;
         float _maxy = -FLT_MAX;
         for (unsigned i = 0; i < vvertex.size(); i++)
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

void Flasher::Render(Sur * const psur)
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
   bool fDrawDragpoints = ((m_selectstate != eNotSelected) || g_pvp->m_fAlwaysDrawDragPoints);

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
   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
      psur->SetFillColor(-1);
      psur->SetBorderColor(RGB(255, 0, 0), false, 0);

      if (pdp->m_fDragging)
      {
         psur->SetBorderColor(RGB(0, 255, 0), false, 0);
      }
      if (fDrawDragpoints)
      {
         psur->SetObject(pdp);
         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}


void Flasher::RenderBlueprint(Sur *psur, const bool solid)
{
}

void Flasher::GetTimers(Vector<HitTimer> * const pvht)
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

void Flasher::GetHitShapes(Vector<HitObject> * const pvho)
{
}

void Flasher::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}


void Flasher::EndPlay()
{
   IEditable::EndPlay();

   if (dynamicVertexBuffer)
   {
      dynamicVertexBuffer->release();
      dynamicVertexBuffer = 0;
      dynamicVertexBufferRegenerate = true;
   }
   if (dynamicIndexBuffer)
   {
      dynamicIndexBuffer->release();
      dynamicIndexBuffer = 0;
   }
   if (vertices)
   {
      delete[] vertices;
      vertices = 0;
   }
}

void Flasher::UpdateMesh()
{
   const float height = (m_d.m_height + m_ptable->m_tableheight)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   const float movx = minx + (maxx - minx)*0.5f;
   const float movy = miny + (maxy - miny)*0.5f;

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
   dynamicVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);

   for (unsigned int i = 0; i < numVertices; i++)
   {
      Vertex3D_TexelOnly vert = vertices[i];
      tempMatrix.MultiplyVector(vert, vert);
      buf[i] = vert;
   }

   dynamicVertexBuffer->unlock();
}

void Flasher::RenderSetup(RenderDevice* pd3dDevice)
{
   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);

   numVertices = (unsigned int)vvertex.size();

   std::vector<WORD> vtri;
   
   {
   std::vector<unsigned int> vpoly(numVertices);
   for (unsigned int i = 0; i < numVertices; i++)
      vpoly[i] = i;

   PolygonToTriangles(vvertex, vpoly, vtri);
   }

   numPolys = (int)(vtri.size()/3);
   if (numPolys == 0)
   {
      // no polys to render leave vertex buffer undefined 
      return;
   }

   if (dynamicIndexBuffer)
      dynamicIndexBuffer->release();
   pd3dDevice->CreateIndexBuffer(numPolys * 3, 0, IndexBuffer::FMT_INDEX16, &dynamicIndexBuffer);
   NumVideoBytes += numPolys * 3 * sizeof(WORD);

   WORD* bufi;
   dynamicIndexBuffer->lock(0, 0, (void**)&bufi, 0);
   memcpy(bufi, vtri.data(), vtri.size()*sizeof(WORD));
   dynamicIndexBuffer->unlock();

   if (dynamicVertexBuffer)
      dynamicVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(numVertices, USAGE_DYNAMIC, MY_D3DFVF_TEX, &dynamicVertexBuffer);
   NumVideoBytes += (int)(numVertices*sizeof(Vertex3D_TexelOnly));

   if (vertices)
      delete[] vertices;
   vertices = new Vertex3D_TexelOnly[numVertices];

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   minx = FLT_MAX;
   miny = FLT_MAX;
   maxx = -FLT_MAX;
   maxy = -FLT_MAX;

   for (unsigned int i = 0; i < numVertices; i++)
   {
      const RenderVertex * const pv0 = &vvertex[i];

      vertices[i].x = pv0->x;
      vertices[i].y = pv0->y;
      vertices[i].z = 0;

      if (pv0->x > maxx) maxx = pv0->x;
      if (pv0->x < minx) minx = pv0->x;
      if (pv0->y > maxy) maxy = pv0->y;
      if (pv0->y < miny) miny = pv0->y;
   }

   const float inv_width = 1.0f / (maxx - minx);
   const float inv_height = 1.0f / (maxy - miny);
   const float inv_tablewidth = 1.0f / (m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f / (m_ptable->m_bottom - m_ptable->m_top);
   m_d.m_vCenter.x = minx + ((maxx - minx)*0.5f);
   m_d.m_vCenter.y = miny + ((maxy - miny)*0.5f);

   for (unsigned int i = 0; i < numVertices; i++)
   {
      if (m_d.m_imagealignment == ImageModeWrap)
      {
         vertices[i].tu = (vertices[i].x - minx)*inv_width;
         vertices[i].tv = (vertices[i].y - miny)*inv_height;
      }
      else
      {
         vertices[i].tu = vertices[i].x*inv_tablewidth;
         vertices[i].tv = vertices[i].y*inv_tableheight;
      }
   }
}

void Flasher::RenderStatic(RenderDevice* pd3dDevice)
{
}

void Flasher::SetObjectPos()
{
   g_pvp->SetObjectPosCur(0, 0);
}

void Flasher::FlipY(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Flasher::FlipX(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Flasher::Rotate(float ang, Vertex2D *pvCenter, const bool useElementCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter, useElementCenter);
}

void Flasher::Scale(float scalex, float scaley, Vertex2D *pvCenter, const bool useElementsCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter, useElementsCenter);
}

void Flasher::Translate(Vertex2D *pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

void Flasher::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;
   for (int i = 0; i < m_vdpoint.Size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }

   m_ptable->SetDirtyDraw();
}

void Flasher::DoCommand(int icmd, int x, int y)
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
      for (int i = 0; i < (iSeg + 1); i++)
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

HRESULT Flasher::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteFloat(FID(FHEI), m_d.m_height);
   bw.WriteFloat(FID(FLAX), m_d.m_vCenter.x);
   bw.WriteFloat(FID(FLAY), m_d.m_vCenter.y);
   bw.WriteFloat(FID(FROX), m_d.m_rotX);
   bw.WriteFloat(FID(FROY), m_d.m_rotY);
   bw.WriteFloat(FID(FROZ), m_d.m_rotZ);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(IMAG), m_d.m_szImageA);
   bw.WriteString(FID(IMAB), m_d.m_szImageB);
   bw.WriteInt(FID(FALP), m_d.m_fAlpha);
   bw.WriteFloat(FID(MOVA), m_d.m_modulate_vs_add);
   bw.WriteBool(FID(FVIS), m_d.m_IsVisible);
   bw.WriteBool(FID(DSPT), m_d.m_fDisplayTexture);
   bw.WriteBool(FID(ADDB), m_d.m_fAddBlend);
   bw.WriteBool(FID(IDMD), m_d.m_IsDMD);
   bw.WriteFloat(FID(FLDB), m_d.m_depthBias);
   bw.WriteInt(FID(ALGN), m_d.m_imagealignment);
   bw.WriteInt(FID(FILT), m_d.m_filter);
   bw.WriteInt(FID(FIAM), m_d.m_fFilterAmount);
   ISelect::SaveData(pstm, hcrypthash, hcryptkey);
   HRESULT hr;
   if (FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
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

BOOL Flasher::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(FHEI))
   {
      pbr->GetFloat(&m_d.m_height);
   }
   else if (id == FID(FLAX))
   {
      pbr->GetFloat(&m_d.m_vCenter.x);
   }
   else if (id == FID(FLAY))
   {
      pbr->GetFloat(&m_d.m_vCenter.y);
   }
   else if (id == FID(FROX))
   {
      pbr->GetFloat(&m_d.m_rotX);
   }
   else if (id == FID(FROY))
   {
      pbr->GetFloat(&m_d.m_rotY);
   }
   else if (id == FID(FROZ))
   {
      pbr->GetFloat(&m_d.m_rotZ);
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
      pbr->GetString(m_d.m_szImageA);
   }
   else if (id == FID(IMAB))
   {
      pbr->GetString(m_d.m_szImageB);
   }
   else if (id == FID(FALP))
   {
      int iTmp;
      pbr->GetInt(&iTmp);
      //if( iTmp>100 ) iTmp=100;
      if (iTmp < 0) iTmp = 0;
      m_d.m_fAlpha = iTmp;
   }
   else if (id == FID(MOVA))
   {
      pbr->GetFloat(&m_d.m_modulate_vs_add);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(FVIS))
   {
      BOOL iTmp;
      pbr->GetBool(&iTmp);
      m_d.m_IsVisible = (iTmp == 1);
   }
   else if (id == FID(ADDB))
   {
      BOOL iTmp;
      pbr->GetBool(&iTmp);
      m_d.m_fAddBlend = (iTmp == 1);
   }
   else if (id == FID(IDMD))
   {
	   BOOL iTmp;
	   pbr->GetBool(&iTmp);
	   m_d.m_IsDMD = (iTmp == 1);
   }
   else if (id == FID(DSPT))
   {
      pbr->GetBool(&m_d.m_fDisplayTexture);
   }
   else if (id == FID(FLDB))
   {
      pbr->GetFloat(&m_d.m_depthBias);
   }
   else if (id == FID(ALGN))
   {
      pbr->GetInt(&m_d.m_imagealignment);
   }
   else if (id == FID(FILT))
   {
      pbr->GetInt(&m_d.m_filter);
   }
   else if (id == FID(FIAM))
   {
      pbr->GetInt(&m_d.m_fFilterAmount);
   }
   else
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
   }
   return fTrue;
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

   return S_OK;
}

STDMETHODIMP Flasher::put_X(float newVal)
{
   if (m_d.m_vCenter.x != newVal)
   {
      STARTUNDO

         m_d.m_vCenter.x = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
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
      STARTUNDO

         m_d.m_vCenter.y = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
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
      STARTUNDO

         m_d.m_rotX = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
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
      STARTUNDO

         m_d.m_rotY = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
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
      STARTUNDO

         m_d.m_rotZ = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
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
      STARTUNDO

         m_d.m_height = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
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
      STARTUNDO

         m_d.m_color = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}

void Flasher::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   m_propVisual = new PropertyPane(IDD_PROPFLASHER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(m_propVisual);

   pproppane = new PropertyPane(IDD_PROPFLASHER_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

void Flasher::GetPointDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROPPOINT_VISUALSWTEX, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPPOINT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Flasher::get_ImageA(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImageA, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_ImageA(BSTR newVal)
{
   char m_szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szImage, 32, NULL, NULL);

   if (strcmp(m_szImage, m_d.m_szImageA) != 0)
   {
      STARTUNDO

      strcpy_s(m_d.m_szImageA, MAXTOKEN, m_szImage);

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_ImageB(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImageB, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_ImageB(BSTR newVal)
{
   char m_szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szImage, 32, NULL, NULL);

   if (strcmp(m_szImage, m_d.m_szImageB) != 0)
   {
      STARTUNDO

      strcpy_s(m_d.m_szImageB, MAXTOKEN, m_szImage);

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Filter(BSTR *pVal)
{
   WCHAR wz[512];

   switch (m_d.m_filter)
   {
   case Filter_Additive:
   {
      MultiByteToWideChar(CP_ACP, 0, "Additive", -1, wz, 32);
      break;
   }
   case Filter_Multiply:
   {
      MultiByteToWideChar(CP_ACP, 0, "Multiply", -1, wz, 32);
      break;
   }
   case Filter_Overlay:
   {
      MultiByteToWideChar(CP_ACP, 0, "Overlay", -1, wz, 32);
      break;
   }
   case Filter_Screen:
   {
      MultiByteToWideChar(CP_ACP, 0, "Screen", -1, wz, 32);
      break;
   }
   default:
   case Filter_None:
   {
      MultiByteToWideChar(CP_ACP, 0, "None", -1, wz, 32);
      break;
   }
   }
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_Filter(BSTR newVal)
{
   char m_szFilter[32];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szFilter, 32, NULL, NULL);

   if (strcmp(m_szFilter, "Additive") == 0 && m_d.m_filter != Filter_Additive)
   {
      STARTUNDO
         m_d.m_filter = Filter_Additive;
      STOPUNDO
         dynamicVertexBufferRegenerate = true;
   }
   else if (strcmp(m_szFilter, "Multiply") == 0 && m_d.m_filter != Filter_Multiply)
   {
      STARTUNDO
         m_d.m_filter = Filter_Multiply;
      STOPUNDO
         dynamicVertexBufferRegenerate = true;
   }
   else if (strcmp(m_szFilter, "Overlay") == 0 && m_d.m_filter != Filter_Overlay)
   {
      STARTUNDO
         m_d.m_filter = Filter_Overlay;
      STOPUNDO
         dynamicVertexBufferRegenerate = true;
   }
   else if (strcmp(m_szFilter, "Screen") == 0 && m_d.m_filter != Filter_Screen)
   {
      STARTUNDO
         m_d.m_filter = Filter_Screen;
      STOPUNDO
         dynamicVertexBufferRegenerate = true;
   }
   else if (strcmp(m_szFilter, "None") == 0 && m_d.m_filter != Filter_None)
   {
      STARTUNDO
         m_d.m_filter = Filter_None;
      STOPUNDO
         dynamicVertexBufferRegenerate = true;
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Opacity(long *pVal)
{
   *pVal = m_d.m_fAlpha;
   return S_OK;
}

STDMETHODIMP Flasher::put_Opacity(long newVal)
{
   STARTUNDO

      m_d.m_fAlpha = newVal;
   //if (m_d.m_fAlpha>100 ) m_d.m_fAlpha=100;
   if (m_d.m_fAlpha < 0) m_d.m_fAlpha = 0;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Flasher::get_IntensityScale(float *pVal)
{
   *pVal = m_d.m_intensity_scale;
   return S_OK;
}

STDMETHODIMP Flasher::put_IntensityScale(float newVal)
{
   STARTUNDO

      m_d.m_intensity_scale = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Flasher::get_ModulateVsAdd(float *pVal)
{
   *pVal = m_d.m_modulate_vs_add;
   return S_OK;
}

STDMETHODIMP Flasher::put_ModulateVsAdd(float newVal)
{
   STARTUNDO

      m_d.m_modulate_vs_add = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Flasher::get_Amount(long *pVal)
{
   *pVal = m_d.m_fFilterAmount;
   return S_OK;
}

STDMETHODIMP Flasher::put_Amount(long newVal)
{
   STARTUNDO

      m_d.m_fFilterAmount = newVal;
   //if (m_d.m_fFilterAmount>100 ) m_d.m_fFilterAmount=100;
   if (m_d.m_fFilterAmount < 0) m_d.m_fFilterAmount = 0;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Flasher::get_Visible(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_IsVisible);

   return S_OK;
}

STDMETHODIMP Flasher::put_Visible(VARIANT_BOOL newVal)
{
   //   if (!g_pplayer )
   {
      STARTUNDO

         m_d.m_IsVisible = VBTOF(newVal);			// set visibility
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_DisplayTexture(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fDisplayTexture);

   return S_OK;
}

STDMETHODIMP Flasher::put_DisplayTexture(VARIANT_BOOL newVal)
{
   if (!g_pplayer)
   {
      STARTUNDO

         m_d.m_fDisplayTexture = VBTOF(newVal);			// set visibility

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_AddBlend(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fAddBlend);

   return S_OK;
}

STDMETHODIMP Flasher::put_AddBlend(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fAddBlend = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flasher::get_DMD(VARIANT_BOOL *pVal) //temporary value of object
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_IsDMD);

	return S_OK;
}

STDMETHODIMP Flasher::put_DMD(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_IsDMD = VBTOF(newVal);

	STOPUNDO

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
      STARTUNDO

         m_d.m_depthBias = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
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
      STARTUNDO

      m_d.m_imagealignment = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}

// Always called each frame to render over everything else (along with primitives)
void Flasher::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   // Don't render if invisible (or DMD connection not set)
   if (!m_d.m_IsVisible || dynamicVertexBuffer == NULL || m_ptable->m_fReflectionEnabled || (m_d.m_IsDMD && !g_pplayer->m_texdmd))
      return;

   const D3DXVECTOR4 color = convertColor(m_d.m_color, (float)m_d.m_fAlpha*m_d.m_intensity_scale / 100.0f);
   if (color.w == 0.f)
      return;

   if (color.x == 0.f && color.y == 0.f && color.z == 0.f)
      return;

   if (m_d.m_IsDMD && (g_pplayer->m_dmdstate == 0)) // don't draw any DMD, but this case should not happen in the first place
      return;

   const bool alphadmd = (m_d.m_modulate_vs_add < 1.f);

   if (m_d.m_IsDMD &&
       (((g_pplayer->m_dmdstate == 1) && alphadmd) || // render alpha DMD
        ((g_pplayer->m_dmdstate == 2) && !alphadmd))) // render normal DMD
   {
       if (dynamicVertexBufferRegenerate)
       {
         UpdateMesh();
         dynamicVertexBufferRegenerate = false;
       }

       pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
       pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);

       pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
       if((g_pplayer->m_dmdstate == 1) && alphadmd)
          g_pplayer->m_pin3d.EnableAlphaBlend(m_d.m_fAddBlend);
       else
          g_pplayer->m_pin3d.DisableAlphaBlend();

       /*const unsigned int alphamode = 1; //!! make configurable?
       // add
       if (alphamode == 1) {
         g_pplayer->m_pin3d.EnableAlphaBlend(true);
       // max
       } else if (alphamode == 2) {
         pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
         pd3dDevice->SetRenderState(RenderDevice::BLENDOP, D3DBLENDOP_MAX);
         pd3dDevice->SetRenderState(RenderDevice::SRCBLEND, D3DBLEND_SRCALPHA);
         pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCCOLOR);
       //subtract
       } else if (alphamode == 3) {
         pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
         pd3dDevice->SetRenderState(RenderDevice::BLENDOP, D3DBLENDOP_SUBTRACT);
         pd3dDevice->SetRenderState(RenderDevice::SRCBLEND, D3DBLEND_ZERO);
         pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCCOLOR);
       // normal
       } else if (alphamode == 4) {
         pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
         pd3dDevice->SetRenderState(RenderDevice::BLENDOP, D3DBLENDOP_ADD);
         pd3dDevice->SetRenderState(RenderDevice::SRCBLEND, D3DBLEND_SRCALPHA);
         pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA);
       }

       const int alphatest = 0; //!!
       if (alphatest)
         g_pplayer->m_pin3d.EnableAlphaTestReference(0x80);*/

       //const float width = g_pplayer->m_pin3d.m_useAA ? 2.0f*(float)m_width : (float)m_width;
       pd3dDevice->DMDShader->SetTechnique("basic_DMD_tiny_world"); //width*DMDwidth / (float)m_dmdx <= 3.74f ? "basic_DMD_tiny" : (width*DMDwidth / (float)m_dmdx <= 7.49f ? "basic_DMD" : "basic_DMD_big")); // use different smoothing functions for LED/Plasma emulation (rule of thumb here: up to quarter width of 1920HD = tiny, up to half width of 1920HD = normal, up to full width of 1920HD = big)

       pd3dDevice->DMDShader->SetVector("vColor_Intensity", &color);

#ifdef DMD_UPSCALE
       const D3DXVECTOR4 r((float)(g_pplayer->m_dmdx*3), (float)(g_pplayer->m_dmdy*3), m_d.m_modulate_vs_add, 1.f); //(float)(0.5 / m_width), (float)(0.5 / m_height));
#else
       const D3DXVECTOR4 r((float)g_pplayer->m_dmdx, (float)g_pplayer->m_dmdy, m_d.m_modulate_vs_add, 1.f); //(float)(0.5 / m_width), (float)(0.5 / m_height));
#endif
       pd3dDevice->DMDShader->SetVector("vRes_Alpha", &r);

       pd3dDevice->DMDShader->SetTexture("Texture0", g_pplayer->m_pin3d.m_pd3dDevice->m_texMan.LoadTexture(g_pplayer->m_texdmd));

       pd3dDevice->DMDShader->Begin(0);
       pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_TEX, dynamicVertexBuffer, 0, numVertices, dynamicIndexBuffer, 0, numPolys * 3);
       pd3dDevice->DMDShader->End();
   }
   else if (g_pplayer->m_dmdstate == 0)
   {
       if (dynamicVertexBufferRegenerate)
       {
          UpdateMesh();
          dynamicVertexBufferRegenerate = false;
       }

       pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
       pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);

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

       const D3DXVECTOR4 ab((float)m_d.m_fFilterAmount / 100.0f, min(max(m_d.m_modulate_vs_add, 0.00001f), 0.9999f), // avoid 0, as it disables the blend and avoid 1 as it looks not good with day->night changes
           hdrTex0 ? 1.f : 0.f, (pinA && pinB && pinB->IsHDR()) ? 1.f : 0.f);
       pd3dDevice->flasherShader->SetVector("amount__blend_modulate_vs_add__hdrTexture01", &ab);

       pd3dDevice->flasherShader->SetFlasherColorAlpha(color);

       D3DXVECTOR4 flasherData(-1.f, -1.f, (float)m_d.m_filter, m_d.m_fAddBlend ? 1.f : 0.f);

       if (pinA && !pinB)
       {
           pd3dDevice->flasherShader->SetTechnique("basic_with_textureOne_noLight");
           pd3dDevice->flasherShader->SetTexture("Texture0", pinA);

           if (!m_d.m_fAddBlend)
               flasherData.x = pinA->m_alphaTestValue * (float)(1.0 / 255.0);

           //ppin3d->SetTextureFilter( 0, TEXTURE_MODE_TRILINEAR );
       }
       else if (!pinA && pinB)
       {
           pd3dDevice->flasherShader->SetTechnique("basic_with_textureOne_noLight");
           pd3dDevice->flasherShader->SetTexture("Texture0", pinB);

           if (!m_d.m_fAddBlend)
               flasherData.x = pinB->m_alphaTestValue * (float)(1.0 / 255.0);

           //ppin3d->SetTextureFilter( 0, TEXTURE_MODE_TRILINEAR );
       }
       else if (pinA && pinB)
       {
           pd3dDevice->flasherShader->SetTechnique("basic_with_textureAB_noLight");
           pd3dDevice->flasherShader->SetTexture("Texture0", pinA);
           pd3dDevice->flasherShader->SetTexture("Texture1", pinB);

           if (!m_d.m_fAddBlend)
           {
               flasherData.x = pinA->m_alphaTestValue * (float)(1.0 / 255.0);
               flasherData.y = pinB->m_alphaTestValue * (float)(1.0 / 255.0);
           }

           //ppin3d->SetTextureFilter( 0, TEXTURE_MODE_TRILINEAR );
       }
       else
           pd3dDevice->flasherShader->SetTechnique("basic_with_noLight");

       pd3dDevice->flasherShader->SetFlasherData(flasherData);

       pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
       g_pplayer->m_pin3d.EnableAlphaBlend(m_d.m_fAddBlend, false, false);
       pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, m_d.m_fAddBlend ? D3DBLEND_INVSRCCOLOR : D3DBLEND_INVSRCALPHA);
       pd3dDevice->SetRenderState(RenderDevice::BLENDOP, m_d.m_fAddBlend ? D3DBLENDOP_REVSUBTRACT : D3DBLENDOP_ADD);
       
       pd3dDevice->flasherShader->Begin(0);
       pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_TEX, dynamicVertexBuffer, 0, numVertices, dynamicIndexBuffer, 0, numPolys * 3);
       pd3dDevice->flasherShader->End();
   }

   //pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
   //pd3dDevice->SetRenderState(RenderDevice::BLENDOP, D3DBLENDOP_ADD); //!! not necessary anymore
   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
}

void Flasher::UpdatePropertyPanes()
{
   if (m_propVisual == NULL)
      return;

   if (m_d.m_szImageA[0] == 0 && m_d.m_szImageB[0] == 0)
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, 8), FALSE);
   else
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, 8), TRUE);

   if (m_d.m_szImageB[0] == 0 || m_d.m_szImageA[0] == 0)
   {
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_EFFECT_COMBO), FALSE);
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_FILTERAMOUNT_EDIT), FALSE);
   }
   else
   {
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_EFFECT_COMBO), TRUE);
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_FILTERAMOUNT_EDIT), TRUE);
   }
}
