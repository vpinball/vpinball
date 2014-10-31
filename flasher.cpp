#include "StdAfx.h"

Flasher::Flasher()
{
   m_menuid = IDR_SURFACEMENU;
   m_d.m_IsVisible = true;
   m_d.m_depthBias = 0.0f;
   dynamicVertexBuffer = 0;
   dynamicVertexBufferRegenerate = true;
}

Flasher::~Flasher()
{
	if(dynamicVertexBuffer) {
		dynamicVertexBuffer->release();
		dynamicVertexBuffer = 0;
	}
}

void Flasher::InitShape()
{
   if ( m_vdpoint.Size() == 0 )
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
         pdp->Init(this, x-size*0.5f, y-size*0.5f);
         pdp->m_fSmooth = FALSE;
         m_vdpoint.AddElement(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x-size*0.5f, y+size*0.5f);
         pdp->m_fSmooth = FALSE;
         m_vdpoint.AddElement(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x+size*0.5f, y+size*0.5f);
         pdp->m_fSmooth = FALSE;
         m_vdpoint.AddElement(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x+size*0.5f, y-size*0.5f);
         pdp->m_fSmooth = FALSE;
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

   m_d.m_rotX=0.0f;
   m_d.m_rotY=0.0f;
   m_d.m_rotZ=0.0f;
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

   hr = GetRegStringAsFloat("DefaultProps\\Flasher","Height", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_height = fTmp;
   else
      m_d.m_height = 50.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flasher","RotX", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotX = fTmp;
   else
      m_d.m_rotX = 0.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flasher","RotY", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotY = fTmp;
   else
      m_d.m_rotY = 0.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flasher","RotZ", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotZ = fTmp;
   else
      m_d.m_rotZ = 0.0f;

   hr = GetRegInt("DefaultProps\\Flasher","Color", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_color = iTmp;
   else
      m_d.m_color = RGB(50,200,50);

   hr = GetRegInt("DefaultProps\\Flasher","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? fFalse : fTrue;
   else
      m_d.m_tdr.m_fTimerEnabled = fFalse;

   hr = GetRegInt("DefaultProps\\Flasher","TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegString("DefaultProps\\Flasher","ImageA", m_d.m_szImageA, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageA[0] = 0;

   hr = GetRegString("DefaultProps\\Flasher","ImageB", m_d.m_szImageB, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageB[0] = 0;

   hr = GetRegInt("DefaultProps\\Flasher","Alpha", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_fAlpha = iTmp;
   else
       m_d.m_fAlpha = 255;

   hr = GetRegInt("DefaultProps\\Flasher","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_IsVisible = iTmp == 0 ? false : true;
   else
      m_d.m_IsVisible = true;

   hr = GetRegInt("DefaultProps\\Flasher","AddBlend", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_fAddBlend = iTmp == 0 ? false : true;
   else
       m_d.m_fAddBlend = false;

   hr = GetRegInt("DefaultProps\\Flasher","DisplayTexture", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_fDisplayTexture = (iTmp == 0) ? false : true;
   else
       m_d.m_fDisplayTexture = false;

   m_d.m_imagealignment = fromMouseClick ? (RampImageAlignment)GetRegIntWithDefault("DefaultProps\\Flasher","ImageMode", ImageModeWrap) : ImageModeWrap;
   m_d.m_filter = fromMouseClick ? (Filters)GetRegIntWithDefault("DefaultProps\\Flasher","Filter", Filter_Overlay) : Filter_Overlay;
}

void Flasher::WriteRegDefaults()
{
   SetRegValueFloat("DefaultProps\\Flasher","Height", m_d.m_height);
   SetRegValueFloat("DefaultProps\\Flasher","RotX", m_d.m_rotX);
   SetRegValueFloat("DefaultProps\\Flasher","RotY", m_d.m_rotY);
   SetRegValueFloat("DefaultProps\\Flasher","RotZ", m_d.m_rotZ);
   SetRegValue("DefaultProps\\Flasher","Color",REG_DWORD,&m_d.m_color,4);
   SetRegValueBool("DefaultProps\\Flasher","TimerEnabled",!!m_d.m_tdr.m_fTimerEnabled);
   SetRegValueInt("DefaultProps\\Flasher","TimerInterval",m_d.m_tdr.m_TimerInterval);
   SetRegValue("DefaultProps\\Flasher","ImageA", REG_SZ, &m_d.m_szImageA, lstrlen(m_d.m_szImageA));
   SetRegValue("DefaultProps\\Flasher","ImageB", REG_SZ, &m_d.m_szImageB, lstrlen(m_d.m_szImageB));
   SetRegValueInt("DefaultProps\\Flasher","Alpha",m_d.m_fAlpha);
   SetRegValueBool("DefaultProps\\Flasher","Visible",m_d.m_IsVisible);
   SetRegValueBool("DefaultProps\\Flasher","DisplayTexture",m_d.m_fDisplayTexture);
   SetRegValueBool("DefaultProps\\Flasher","AddBlend",m_d.m_fAddBlend);
   SetRegValue("DefaultProps\\Flasher","ImageMode",REG_DWORD,&m_d.m_imagealignment,4);
   SetRegValue("DefaultProps\\Flasher","Filter",REG_DWORD,&m_d.m_filter,4);
}

void Flasher::PreRender(Sur * const psur)
{
   if ( m_vdpoint.Size() == 0 )
   InitShape();

   psur->SetFillColor(m_ptable->RenderSolid() ? RGB(192,192,192) : -1);
   psur->SetObject(this);
   // Don't want border color to be over-ridden when selected - that will be drawn later
   psur->SetBorderColor(-1,false,0);

   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);
   Texture *ppi;
   if (m_ptable->RenderSolid() && m_d.m_fDisplayTexture && (ppi = m_ptable->GetImage(m_d.m_szImageA)))
   {
      ppi->EnsureHBitmap();
      if ( m_d.m_imagealignment == ImageModeWrap )
      {
         float minx=FLT_MAX;
         float miny=FLT_MAX;
         float maxx=-FLT_MAX;
         float maxy=-FLT_MAX;
         for( int i=0;i<vvertex.Size();i++ )
         {
            if( vvertex.ElementAt(i)->x<minx) minx=vvertex.ElementAt(i)->x;
            if( vvertex.ElementAt(i)->x>maxx) maxx=vvertex.ElementAt(i)->x;
            if( vvertex.ElementAt(i)->y<miny) miny=vvertex.ElementAt(i)->y;
            if( vvertex.ElementAt(i)->y>maxy) maxy=vvertex.ElementAt(i)->y;
         }
         if (ppi->m_hbmGDIVersion)
            psur->PolygonImage(vvertex, ppi->m_hbmGDIVersion, minx, miny, minx+(maxx-minx), miny+(maxy-miny), ppi->m_width, ppi->m_height);
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

   for (int i=0;i<vvertex.Size();i++) //!! keep for render()
      delete vvertex.ElementAt(i);
}

void Flasher::Render(Sur * const psur)
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
         psur->SetBorderColor(RGB(0,255,0),false,0);
      }
      if (fDrawDragpoints)
      {
         psur->SetObject(pdp);
         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}


void Flasher::RenderBlueprint(Sur *psur)
{
}

void Flasher::GetTimers(Vector<HitTimer> * const pvht)
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

void Flasher::GetHitShapes(Vector<HitObject> * const pvho)
{
}

void Flasher::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}


void Flasher::EndPlay()
{
    IEditable::EndPlay();

	if(dynamicVertexBuffer) 
    {
		dynamicVertexBuffer->release();
		dynamicVertexBuffer = 0;
		dynamicVertexBufferRegenerate = true;
	}
   if( vertices )
   {
      delete[] vertices;
      vertices=0;
   }
}

void Flasher::UpdateMesh()
{
   Vertex3D_TexelOnly *buf;
   dynamicVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   Vertex3D_TexelOnly verts[3];

   const float height = m_d.m_height*m_ptable->m_zScale;
   const float movx=minx+((maxx-minx)*0.5f);
   const float movy=miny+((maxy-miny)*0.5f);

   int offset=0;
   for (int i=0;i<numPolys;i++, offset+=3)
   {
      Matrix3D tempMatrix,RTmatrix,TMatrix,T2Matrix;
      RTmatrix.SetIdentity();
      TMatrix.SetIdentity();
      T2Matrix.SetIdentity();
      T2Matrix._41 = -movx;//-m_d.m_vCenter.x;
      T2Matrix._42 = -movy;//-m_d.m_vCenter.y;
      T2Matrix._43 = 0;
      TMatrix._41 = movx;//m_d.m_vCenter.x;
      TMatrix._42 = movy;//m_d.m_vCenter.y;
      TMatrix._43 = height;

      tempMatrix.SetIdentity();
      tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
      tempMatrix.Multiply(RTmatrix, RTmatrix);
      tempMatrix.RotateYMatrix(ANGTORAD(m_d.m_rotY));
      tempMatrix.Multiply(RTmatrix, RTmatrix);
      tempMatrix.RotateXMatrix(ANGTORAD(m_d.m_rotX));
      tempMatrix.Multiply(RTmatrix, RTmatrix);
      for( int i=0;i<3;i++ )
      {      
         memcpy( &verts[i], &vertices[offset+i], sizeof(Vertex3D_TexelOnly));
         T2Matrix.MultiplyVector(verts[i], verts[i]);
         RTmatrix.MultiplyVector(verts[i], verts[i]);
         TMatrix.MultiplyVector(verts[i], verts[i]);
      }
      memcpy( &buf[offset], verts, sizeof(Vertex3D_TexelOnly)*3 );
   }

   dynamicVertexBuffer->unlock();
}

void Flasher::RenderSetup(RenderDevice* pd3dDevice)
{
   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);

   numVertices = vvertex.Size();

   VectorVoid vpoly;

   for (int i=0;i<numVertices;i++)
      vpoly.AddElement((void *)i);

   Vector<Triangle> vtri;
   PolygonToTriangles(vvertex, &vpoly, &vtri);

   numPolys = vtri.Size();
   if( numPolys==0 )
   {         
      for (int i=0;i<numVertices;i++)
         delete vvertex.ElementAt(i);

      // no polys to render leave vertex buffer undefined 
      return;
   }
   const float inv_tablewidth = 1.0f/(m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f/(m_ptable->m_bottom - m_ptable->m_top);


   if( dynamicVertexBuffer )
      dynamicVertexBuffer->release();

   pd3dDevice->CreateVertexBuffer( numPolys*3, 0, MY_D3DFVF_TEX, &dynamicVertexBuffer );
   NumVideoBytes += numPolys*3*sizeof(Vertex3D_TexelOnly);     
   
   vertices = new Vertex3D_TexelOnly[numPolys*3];

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   Texture * const pin = m_ptable->GetImage(m_d.m_szImageA);

   minx=FLT_MAX;
   miny=FLT_MAX;
   maxx=-FLT_MAX;
   maxy=-FLT_MAX;
   int offset=0;
   for (int i=0;i<numPolys;i++, offset+=3)
   {
      const Triangle * const ptri = vtri.ElementAt(i);

      const RenderVertex * const pv0 = vvertex.ElementAt(ptri->a);
      const RenderVertex * const pv1 = vvertex.ElementAt(ptri->b);
      const RenderVertex * const pv2 = vvertex.ElementAt(ptri->c);

      {
         vertices[offset  ].x=pv0->x;   vertices[offset  ].y=pv0->y;   vertices[offset  ].z=0;
         if( pv0->x>maxx ) maxx=pv0->x; if( pv0->x<minx ) minx=pv0->x;
         if( pv0->y>maxy ) maxy=pv0->y; if( pv0->y<miny ) miny=pv0->y;
         
         vertices[offset+2].x=pv1->x;   vertices[offset+2].y=pv1->y;   vertices[offset+2].z=0;
         if( pv1->x>maxx ) maxx=pv1->x; if( pv1->x<minx ) minx=pv1->x;
         if( pv1->y>maxy ) maxy=pv1->y; if( pv1->y<miny ) miny=pv1->y;

         vertices[offset+1].x=pv2->x;   vertices[offset+1].y=pv2->y;   vertices[offset+1].z=0;
         if( pv2->x>maxx ) maxx=pv2->x; if( pv2->x<minx ) minx=pv2->x;
         if( pv2->y>maxy ) maxy=pv2->y; if( pv2->y<miny ) miny=pv2->y;
      }
      delete vtri.ElementAt(i);
   }

   float width = maxx-minx;
   float height = maxy-miny;
   for( int i=0;i<numPolys*3;i++)
   {
      if (m_d.m_imagealignment == ImageModeWrap)
      {
         vertices[i].tu = (vertices[i].x-minx)/width;
         vertices[i].tv = (vertices[i].y-miny)/height;
      }
      else
      {
         vertices[i].tu = vertices[i].x*inv_tablewidth;
         vertices[i].tv = vertices[i].y*inv_tableheight;
      }
   }
   for (int i=0;i<numVertices;i++)
      delete vvertex.ElementAt(i);

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

void Flasher::Rotate(float ang, Vertex2D *pvCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter);
}

void Flasher::Scale(float scalex, float scaley, Vertex2D *pvCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
}

void Flasher::Translate(Vertex2D *pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

void Flasher::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;
   for (int i=0;i<m_vdpoint.Size();i++)
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
   bw.WriteBool(FID(FVIS), m_d.m_IsVisible);
   bw.WriteBool(FID(DSPT), m_d.m_fDisplayTexture);
   bw.WriteBool(FID(ADDB), m_d.m_fAddBlend);
   bw.WriteFloat(FID(FLDB), m_d.m_depthBias);
   bw.WriteInt(FID(ALGN), m_d.m_imagealignment);
   bw.WriteInt(FID(FILT), m_d.m_filter);   
   ISelect::SaveData(pstm, hcrypthash, hcryptkey);
   HRESULT hr;
   if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
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
      if( iTmp>100 ) iTmp=100;
      if( iTmp<0 ) iTmp=0;
      m_d.m_fAlpha = iTmp;
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(FVIS))
   {
	  BOOL iTmp;
      pbr->GetBool(&iTmp);
      m_d.m_IsVisible = (iTmp==1);
   }
   else if (id == FID(ADDB))
   {
       BOOL iTmp;
       pbr->GetBool(&iTmp);
       m_d.m_fAddBlend = (iTmp==1);
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

   for (size_t i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
      if (InlineIsEqualGUID(*arr[i],riid))
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
   if(m_d.m_vCenter.x!= newVal)
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
   if(m_d.m_vCenter.y != newVal)
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
   if(m_d.m_rotX != newVal)
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
   if(m_d.m_rotY != newVal)
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
   if(m_d.m_rotZ != newVal)
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
   if(m_d.m_height != newVal)
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
   if(m_d.m_color != newVal)
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

   pproppane = new PropertyPane(IDD_PROPFLASHER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

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

   if(strcmp(m_szImage,m_d.m_szImageA) != 0)
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

   if(strcmp(m_szImage,m_d.m_szImageB) != 0)
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

    switch( m_d.m_filter )
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

   if(strcmp(m_szFilter,"Additive") == 0 && m_d.m_filter!=Filter_Additive)
   {
       STARTUNDO
            m_d.m_filter = Filter_Additive;
       STOPUNDO
       dynamicVertexBufferRegenerate=true;
   }
   else if(strcmp(m_szFilter,"Multiply") == 0 && m_d.m_filter!=Filter_Multiply)
   {
       STARTUNDO
           m_d.m_filter = Filter_Multiply;
       STOPUNDO
           dynamicVertexBufferRegenerate=true;
   }
   else if(strcmp(m_szFilter,"Overlay") == 0 && m_d.m_filter!=Filter_Overlay)
   {
       STARTUNDO
           m_d.m_filter = Filter_Overlay;
       STOPUNDO
           dynamicVertexBufferRegenerate=true;
   }
   else if(strcmp(m_szFilter,"Screen") == 0 && m_d.m_filter!=Filter_Screen)
   {
       STARTUNDO
           m_d.m_filter = Filter_Screen;
       STOPUNDO
           dynamicVertexBufferRegenerate=true;
   }
   else if(strcmp(m_szFilter,"None") == 0 && m_d.m_filter!=Filter_None)
   {
       STARTUNDO
           m_d.m_filter = Filter_None;
       STOPUNDO
           dynamicVertexBufferRegenerate=true;
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
   if (m_d.m_fAlpha>100 ) m_d.m_fAlpha=100;
   if (m_d.m_fAlpha<0 ) m_d.m_fAlpha=0;
   
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
   if (!g_pplayer )
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

STDMETHODIMP Flasher::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Flasher::put_DepthBias(float newVal)
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

STDMETHODIMP Flasher::get_ImageAlignment(RampImageAlignment *pVal)
{
   *pVal = m_d.m_imagealignment;

   return S_OK;
}

STDMETHODIMP Flasher::put_ImageAlignment(RampImageAlignment newVal)
{
   if(m_d.m_imagealignment != newVal)
   {
      STARTUNDO

      m_d.m_imagealignment = newVal;
      dynamicVertexBufferRegenerate = true;

      STOPUNDO
   }

   return S_OK;
}

// Always called each frame to render over everything else (along with primitives)
// Same code as RenderStatic (with the exception of the alpha tests).
// Also has less drawing calls by bundling seperate calls.
void Flasher::PostRenderStatic(RenderDevice* pd3dDevice)
{
    TRACE_FUNCTION();
   // Don't render if invisible.
   if(!m_d.m_IsVisible) 
      return;

   pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexTexelDeclaration );

      Pin3D * const ppin3d = &g_pplayer->m_pin3d;
      Texture * const pinA = m_ptable->GetImage(m_d.m_szImageA);
      Texture * const pinB = m_ptable->GetImage(m_d.m_szImageB);

      const D3DXVECTOR4 color = COLORREF_to_D3DXVECTOR4(m_d.m_color);
      //pd3dDevice->basicShader->Core()->SetFloat("fAlpha",(float)m_d.m_fAlpha/100.0f);
      pd3dDevice->basicShader->Core()->SetFloat("fAlpha",(float)1.0f);
      pd3dDevice->basicShader->Core()->SetFloat("fFilterAmount",(float)m_d.m_fAlpha/100.0f);
      pd3dDevice->basicShader->Core()->SetVector("staticColor",&color);
      pd3dDevice->basicShader->Core()->SetBool("bPerformAlphaTest", true);
      pd3dDevice->basicShader->Core()->SetFloat("fAlphaTestValue", 1.0f/255.0f);
      if( m_d.m_filter == Filter_Additive )
         pd3dDevice->basicShader->Core()->SetBool("bAdditive", true);
      else if( m_d.m_filter == Filter_Overlay )
         pd3dDevice->basicShader->Core()->SetBool("bOverlay", true);
      else if( m_d.m_filter == Filter_Multiply )
         pd3dDevice->basicShader->Core()->SetBool("bMultiply", true);
      else if( m_d.m_filter == Filter_Screen )
         pd3dDevice->basicShader->Core()->SetBool("bScreen", true);

      if (pinA && !pinB)
      {
         pinA->CreateAlphaChannel();
         pd3dDevice->basicShader->SetTexture("Texture0", pinA);
         pd3dDevice->basicShader->Core()->SetTechnique("basic_with_textureOne_noLight");

		   ppin3d->SetTextureFilter( ePictureTexture, TEXTURE_MODE_TRILINEAR );
      }
      else if (!pinA && pinB)
      {
         pinA->CreateAlphaChannel();
         pd3dDevice->basicShader->SetTexture("Texture0", pinB);
         pd3dDevice->basicShader->Core()->SetTechnique("basic_with_textureOne_noLight");

         ppin3d->SetTextureFilter( ePictureTexture, TEXTURE_MODE_TRILINEAR );
      }
      else if (pinA && pinB)
      {
         pinA->CreateAlphaChannel();
         pinB->CreateAlphaChannel();
         pd3dDevice->basicShader->SetTexture("Texture0", pinA);
         pd3dDevice->basicShader->SetTexture("Texture1", pinB);
         pd3dDevice->basicShader->Core()->SetTechnique("basic_with_textureAB_noLight");

         ppin3d->SetTextureFilter( ePictureTexture, TEXTURE_MODE_TRILINEAR );
      }
      else
	   {
         pd3dDevice->basicShader->Core()->SetTechnique("basic_with_noLight");
	   }

      if(dynamicVertexBufferRegenerate)
      {
         dynamicVertexBufferRegenerate = false;
         UpdateMesh();
      }

      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
      if ( m_d.m_fAddBlend )
      {
         pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
         pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
         pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_ONE );
      }
      else
      {
          pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
          pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
          pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA);
      }

      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);

      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawPrimitiveVB( D3DPT_TRIANGLELIST, dynamicVertexBuffer, 0, numPolys*3);
      pd3dDevice->basicShader->End();

      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
      pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE); 	
      pd3dDevice->basicShader->Core()->SetBool("bPerformAlphaTest", false);
}
