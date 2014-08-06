#include "StdAfx.h"

Trigger::Trigger()
{
   m_ptriggerhitcircle = NULL;

   m_hitEnabled = fTrue;
   vertexBuffer = NULL;

   m_menuid = IDR_SURFACEMENU;
   material.setDiffuse( 0.0f, 0.5f, 0.5f, 0.5f );
   material.setAmbient( 0.0f, 0.5f, 0.5f, 0.5f );
   material.setSpecular( 0.0f, 0.0f, 0.0f, 0.0f );
   material.setEmissive( 0.0f, 0.0f, 0.0f, 0.0f );
   material.setPower( 0.0f );
}

Trigger::~Trigger()
{
   if( vertexBuffer )
   {
      vertexBuffer->release();
      vertexBuffer=0;
   }
}

HRESULT Trigger::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   if (m_d.m_shape == ShapeCustom)
      put_Shape(ShapeCustom);

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
      m_d.m_fEnabled = iTmp == 0 ? fFalse : fTrue;
   else
      m_d.m_fEnabled = fTrue;

   hr = GetRegInt("DefaultProps\\Trigger","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? fFalse : fTrue;
   else
      m_d.m_fVisible = fTrue;

   hr = GetRegStringAsFloat("DefaultProps\\Trigger","HitHeight", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_hit_height = fTmp;
   else
      m_d.m_hit_height = 50.0f;

   hr = GetRegInt("DefaultProps\\Trigger","Shape", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_shape = (enum Shape)iTmp;
   else
      m_d.m_shape = ShapeCircle;
   hr = GetRegString("DefaultProps\\Trigger","Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;
}

void Trigger::PreRender(Sur * const psur)
{
   psur->SetBorderColor(-1,false,0);
   psur->SetObject(this);

   switch (m_d.m_shape)
   {
   case ShapeCircle:
   default:
      {
         psur->SetFillColor(-1);
         psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
      }
      break;

   case ShapeCustom:
      {
         psur->SetFillColor(m_ptable->RenderSolid() ? RGB(200,220,200) : -1);
         Vector<RenderVertex> vvertex;
         GetRgVertex(&vvertex);

         psur->Polygon(vvertex);

         for (int i=0;i<vvertex.Size();i++)
            delete vvertex.ElementAt(i);
      }
      break;
   }
}

void Trigger::Render(Sur * const psur)
{
   psur->SetLineColor(RGB(0,0,0), false, 0);
   psur->SetObject(this);
   psur->SetFillColor(-1);

   switch (m_d.m_shape)
   {
   case ShapeCircle:
   default: {
      psur->Line(m_d.m_vCenter.x - m_d.m_radius, m_d.m_vCenter.y, m_d.m_vCenter.x + m_d.m_radius, m_d.m_vCenter.y);
      psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - m_d.m_radius, m_d.m_vCenter.x, m_d.m_vCenter.y + m_d.m_radius);

      const float r2 = m_d.m_radius * (float)sin(M_PI/4.0);

      psur->Line(m_d.m_vCenter.x - r2, m_d.m_vCenter.y - r2, m_d.m_vCenter.x + r2, m_d.m_vCenter.y + r2);
      psur->Line(m_d.m_vCenter.x - r2, m_d.m_vCenter.y + r2, m_d.m_vCenter.x + r2, m_d.m_vCenter.y - r2);
      break;
            }

   case ShapeCustom: {
      Vector<RenderVertex> vvertex;
      GetRgVertex(&vvertex);

      psur->SetObject(NULL);
      psur->SetBorderColor(RGB(0,180,0),false,1);

      psur->Polygon(vvertex);

      for (int i=0;i<vvertex.Size();i++)
         delete vvertex.ElementAt(i);

      break;
                     }
   }

   if (m_d.m_shape == ShapeCustom)
   {
      //>>> added by chris
      bool fDrawDragpoints = (m_selectstate != eNotSelected) || (g_pvp->m_fAlwaysDrawDragPoints);		//>>> added by chris
      // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
      if ( !fDrawDragpoints )
      {
         // if any of the dragpoints of this object are selected then draw all the dragpoints
         for (int i=0;i<m_vdpoint.Size();i++)
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
         for (int i=0;i<m_vdpoint.Size();i++)
         {
            CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
            psur->SetFillColor(-1);
            psur->SetBorderColor(RGB(0,180,0),false,0);
            psur->SetObject(pdp);

            if (pdp->m_fDragging)
            {
               psur->SetBorderColor(RGB(0,255,0),false,0);
            }

            psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
         }
      }
   }
   //<<<
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

   if (m_d.m_shape == ShapeCircle)
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
   {
      CurvesToShapes(pvho);
   }
}

void Trigger::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   m_hitEnabled = m_d.m_fEnabled;
   switch (m_d.m_shape)
   {
   case ShapeCircle:
   default:
      {
         HitObject * const pho = CreateCircularHitPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height + 10, m_d.m_radius, 32);
         pho->m_ObjType = eTrigger;
         pho->m_pObj = (void*)this;

         pvho->AddElement(pho);			
         break;
      }

   case ShapeCustom:
      {
         Vector<RenderVertex> vvertex;
         GetRgVertex(&vvertex);

         const int cvertex = vvertex.Size();
         Vertex3Ds * const rgv3d = new Vertex3Ds[cvertex];

         for (int i=0;i<cvertex;i++)
         {
            rgv3d[i].x = vvertex.ElementAt(i)->x;
            rgv3d[i].y = vvertex.ElementAt(i)->y;
            rgv3d[i].z = height + (float)(PHYS_SKIN*2.0);// + 10.0f;
            delete vvertex.ElementAt(i);
         }

         Hit3DPoly * const ph3dp = new Hit3DPoly(rgv3d, cvertex);
         ph3dp->m_ObjType = eTrigger;
         ph3dp->m_pObj = (void*) this;

         pvho->AddElement(ph3dp);
         //ph3dp->m_fEnabled = fFalse;	//rlc error: disable hit process on polygon body, only trigger edges 
         break;
      }
   }
}

void Trigger::CurvesToShapes(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   //Vector<Vertex2D> vvertex;

   //RenderVertex * const rgv = GetRgRenderVertex(&count);

   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);

   const int count = vvertex.Size();
   RenderVertex * const rgv = new RenderVertex[count];
   Vertex3Ds * const rgv3D = new Vertex3Ds[count];

   for (int i=0;i<count;i++)
   {
      rgv[i] = *vvertex.ElementAt(i);
      delete vvertex.ElementAt(i);
      rgv3D[i].x = rgv[i].x;
      rgv3D[i].y = rgv[i].y;
      rgv3D[i].z = height + (float)(PHYS_SKIN*2.0);// + 50.0f; 
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

   ph3dpoly->m_fVisible = fTrue;
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

   /*Vertex2D vt1, vt2;
   vt1.x = pv1->x - pv2->x;
   vt1.y = pv1->y - pv2->y;

   vt2.x = pv1->x - pv3->x;
   vt2.y = pv1->y - pv3->y;*/
}

void Trigger::EndPlay()
{
   IEditable::EndPlay();

   if( vertexBuffer )
   {
      vertexBuffer->release();
      vertexBuffer=0;
   }

   m_ptriggerhitcircle = NULL;
}

const float rgtriggervertex[][3] = {
   -0.08f, -1.0f, 0,
   0.08f,  -1.0f, 0,
   -0.08f, -0.5f, 5.0f,
   0.08f,  -0.5f, 5.0f,
   -0.08f, 0,     8.0f,
   0.08f,  0,     8.0f,
   -0.08f, 0.5f,  5.0f,
   0.08f,  0.5f,  5.0f,
   -0.08f, 1.0f,  0,
   0.08f,  1.0f,  0
};

static const WORD rgtriggerface[][5] = {
   0,2,4,6,8,
   9,7,5,3,1,
   0,1,3,2,0xFFFF,
   2,3,5,4,0xFFFF,
   4,5,7,6,0xFFFF,
   6,7,9,8,0xFFFF
};


void Trigger::PostRenderStatic(RenderDevice* pd3dDevice)
{
}

void Trigger::RenderSetup(RenderDevice* pd3dDevice)
{
   if (!m_d.m_fVisible || m_d.m_shape == ShapeCustom)
      return;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   Vertex3D staticVertices[40];

   for (int i=0;i<4;i++)
   {
      const float angle = (float)(M_PI*2.0/8.0)*(float)i;

      const float sn = sinf(angle);
      const float cs = cosf(angle);

      const int offset = i*10;

      for (int l=0;l<10;l++)
      {
         const float x = rgtriggervertex[l][0]*m_d.m_radius;
         const float y = rgtriggervertex[l][1]*m_d.m_radius;
         staticVertices[l+offset].z = rgtriggervertex[l][2] + height + 0.1f;
         staticVertices[l+offset].z *= m_ptable->m_zScale;

         staticVertices[l+offset].x =  cs*x + sn*y + m_d.m_vCenter.x;
         staticVertices[l+offset].y = -sn*x + cs*y + m_d.m_vCenter.y;
      }
   }
   ppin3d->CalcShadowCoordinates(staticVertices,40);

   for (int i=0;i<4;i++)
   {
      const int offset = i*10;

      for (int l=0;l<6;l++)
      {
         const WORD rgi[5] = 
         {
            rgtriggerface[l][0],
            rgtriggerface[l][1],
            rgtriggerface[l][2],
            rgtriggerface[l][3],
            rgtriggerface[l][4]
         };
         const int cpt = (rgtriggerface[l][4] == 0xFFFF) ? 4 : 5;
         SetNormal(&staticVertices[offset], rgi, cpt, NULL, NULL, 0);
      }
   }

   if ( vertexBuffer==NULL )
   {
      ppin3d->m_pd3dDevice->CreateVertexBuffer( 40, 0, MY_D3DFVF_VERTEX, &vertexBuffer );
      NumVideoBytes += 40*sizeof(Vertex3D);
   }
   Vertex3D *buf;
   vertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
   memcpy( buf, staticVertices, 40*sizeof(Vertex3D));
   vertexBuffer->unlock();

}

void Trigger::RenderStatic(RenderDevice* pd3dDevice)
{
   if (!m_d.m_fVisible || m_d.m_shape == ShapeCustom)
      return;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   ppin3d->EnableLightMap(height);

   pd3dDevice->SetMaterial(material);

   for (int i=0;i<4;i++)
   {
      const int offset = i*10;

      for (int l=0;l<6;l++)
      {
         const WORD rgi[5] = 
         {
            rgtriggerface[l][0],
            rgtriggerface[l][1],
            rgtriggerface[l][2],
            rgtriggerface[l][3],
            rgtriggerface[l][4]
         };
         const int cpt = (rgtriggerface[l][4] == 0xFFFF) ? 4 : 5;
         pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLEFAN, vertexBuffer, offset, 10, (LPWORD)rgi, cpt);
      }
   }

   ppin3d->DisableLightMap();
}

void Trigger::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Trigger::MoveOffset(const float dx, const float dy)
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
   EnableMenuItem(hmenu, ID_WALLMENU_FLIP, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   EnableMenuItem(hmenu, ID_WALLMENU_MIRROR, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   EnableMenuItem(hmenu, ID_WALLMENU_ROTATE, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   EnableMenuItem(hmenu, ID_WALLMENU_SCALE, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   EnableMenuItem(hmenu, ID_WALLMENU_ADDPOINT, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
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

            RECT rc;
         GetClientRect(m_ptable->m_hwnd, &rc);
         HitSur * const phs = new HitSur(NULL, m_ptable->m_zoom, m_ptable->m_offsetx, m_ptable->m_offsety, rc.right - rc.left, rc.bottom - rc.top, 0, 0, NULL);

         const Vertex2D v = phs->ScreenToSurface(x, y);
         delete phs;

         Vector<RenderVertex> vvertex;
         GetRgVertex(&vvertex);

         const int cvertex = vvertex.Size();

         int iSeg;
         Vertex2D vOut;
         ClosestPointOnPolygon(vvertex, v, &vOut, &iSeg, true);

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
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Trigger::FlipX(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Trigger::Rotate(float ang, Vertex2D *pvCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter);
}

void Trigger::Scale(float scalex, float scaley, Vertex2D *pvCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
}

void Trigger::Translate(Vertex2D *pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

HRESULT Trigger::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(RADI), m_d.m_radius);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteBool(FID(EBLD), m_d.m_fEnabled);
   bw.WriteBool(FID(VSBL), m_d.m_fVisible);
   bw.WriteFloat(FID(THOT), m_d.m_hit_height);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteInt(FID(SHAP), m_d.m_shape);

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
   SetRegValue("DefaultProps\\Trigger","Enabled",REG_DWORD,&m_d.m_fEnabled,4);
   SetRegValue("DefaultProps\\Trigger","Visible",REG_DWORD,&m_d.m_fVisible,4);
   SetRegValueFloat("DefaultProps\\Trigger","HitHeight", m_d.m_hit_height);
   SetRegValueFloat("DefaultProps\\Trigger","Radius", m_d.m_radius);
   SetRegValue("DefaultProps\\Trigger","Shape",REG_DWORD,&m_d.m_shape,4);
   SetRegValue("DefaultProps\\Trigger","Surface", REG_SZ, &m_d.m_szSurface,(DWORD)strlen(m_d.m_szSurface));
}

HRESULT Trigger::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
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

void Trigger::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPTRIGGER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPTRIGGER_STATE, IDS_STATE);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Trigger::get_Shape(Shape *pVal)
{
   *pVal = m_d.m_shape;

   return S_OK;
}

STDMETHODIMP Trigger::put_Shape(Shape newVal)
{
   STARTUNDO

      m_d.m_shape = newVal;

   if (m_d.m_shape == ShapeCircle && m_vdpoint.Size() > 0)
   {
      // Set the center of the trigger back to something useful
      float x = 0.0f;
      float y = 0.0f;
      for (int i=0;i<m_vdpoint.Size();i++)
      {
         x += m_vdpoint.ElementAt(i)->m_v.x;
         y += m_vdpoint.ElementAt(i)->m_v.y;
      }

      const float inv_s = 1.0f/(float)m_vdpoint.Size();
      m_d.m_vCenter.x = x*inv_s;
      m_d.m_vCenter.y = y*inv_s;
   }

   if (m_d.m_shape == ShapeCustom && m_vdpoint.Size() == 0)
   {
      // First time shape has been set to custom - set up some points
      const float x = m_d.m_vCenter.x;
      const float y = m_d.m_vCenter.y;

      CComObject<DragPoint> *pdp;
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x-30.0f, y-30.0f);
         m_vdpoint.AddElement(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x-30.0f, y+30.0f);
         m_vdpoint.AddElement(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x+30.0f, y+30.0f);
         m_vdpoint.AddElement(pdp);
      }
      CComObject<DragPoint>::CreateInstance(&pdp);
      if (pdp)
      {
         pdp->AddRef();
         pdp->Init(this, x+30.0f, y-30.0f);
         m_vdpoint.AddElement(pdp);
      }
   }

   STOPUNDO

      return S_OK;
}
