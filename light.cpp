#include "StdAfx.h"

LightCenter::LightCenter(Light *plight)
{
   m_plight = plight;
}

HRESULT LightCenter::GetTypeName(BSTR *pVal)
{
	return m_plight->GetTypeName(pVal);
}

IDispatch *LightCenter::GetDispatch() 
{
	return m_plight->GetDispatch();
}

void LightCenter::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
	m_plight->GetDialogPanes(pvproppane);
}

void LightCenter::Delete()
{
	m_plight->Delete();
}

void LightCenter::Uncreate()
{
	m_plight->Uncreate();
}

IEditable *LightCenter::GetIEditable()
{
	return (IEditable *)m_plight;
}

PinTable *LightCenter::GetPTable()
{
	return m_plight->GetPTable();
}

void LightCenter::GetCenter(Vertex2D * const pv) const
{
   *pv = m_plight->m_d.m_vCenter;
}

void LightCenter::PutCenter(const Vertex2D * const pv)
{
   m_plight->m_d.m_vCenter = *pv;
}

void LightCenter::MoveOffset(const float dx, const float dy)
{
   m_plight->m_d.m_vCenter.x += dx;
   m_plight->m_d.m_vCenter.y += dy;

   GetPTable()->SetDirtyDraw();
}

int LightCenter::GetSelectLevel()
{
   return (m_plight->m_d.m_shape == ShapeCircle) ? 1 : 2; // Don't select light bulb twice if we have drag points
}


Shader   *Light::m_pInsertShader=NULL;

Light::Light() : m_lightcenter(this)
{
   m_menuid = IDR_SURFACEMENU;
   customVBuffer = NULL;
   normalVBuffer = NULL;
   customMoverVBuffer = NULL;
   normalMoverVBuffer = NULL;
   m_d.m_szOffImage[0]=0;
   m_d.m_szOnImage[0]=0;
   m_d.m_depthBias = 0.0f;
   m_d.m_shape = ShapeCustom;
   m_roundLight=false;
}

Light::~Light()
{
   if( customVBuffer )
   {
      customVBuffer->release();
      customVBuffer=0;
   }
   if( normalVBuffer )
   {
      normalVBuffer->release();
      normalVBuffer=0;
   }
   if( customMoverVBuffer )
   {
      customMoverVBuffer->release();
      customMoverVBuffer=0;
   }
   if( normalMoverVBuffer )
   {
      normalMoverVBuffer->release();
      normalMoverVBuffer=0;
   }
}

HRESULT Light::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   InitShape();

   m_fLockedByLS = false;			//>>> added by chris
   m_realState	= m_d.m_state;		//>>> added by chris

   return InitVBA(fTrue, 0, NULL);
}

void Light::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Light","Falloff", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_falloff = fTmp;
   else
      m_d.m_falloff = 50;

   hr = GetRegInt("DefaultProps\\Light","LightState", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_state = (enum LightState)iTmp;
   else
      m_d.m_state = LightStateOff;

   m_d.m_shape = ShapeCustom;

   hr = GetRegInt("DefaultProps\\Light","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? fFalse : fTrue;
   else
      m_d.m_tdr.m_fTimerEnabled = fFalse;

   hr = GetRegInt("DefaultProps\\Light","TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegInt("DefaultProps\\Light","Color", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_color = iTmp;
   else
      m_d.m_color = RGB(255,255,0);

   hr = GetRegString("DefaultProps\\Light","OffImage", m_d.m_szOffImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szOffImage[0] = 0;

   hr = GetRegString("DefaultProps\\Light","OnImage", m_d.m_szOnImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szOnImage[0] = 0;

   hr = GetRegInt("DefaultProps\\Light","DisplayImage", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fDisplayImage = iTmp == 0 ? false : true;
   else
      m_d.m_fDisplayImage = fFalse;

   hr = GetRegString("DefaultProps\\Light","BlinkPattern", m_rgblinkpattern, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      strcpy_s(m_rgblinkpattern, sizeof(m_rgblinkpattern), "10");

   hr = GetRegInt("DefaultProps\\Light","BlinkInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_blinkinterval = iTmp;
   else
      m_blinkinterval = 125;

   hr = GetRegStringAsFloat("DefaultProps\\Light","Intensity", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_intensity = fTmp;
   else
      m_d.m_intensity = 1.0f;

   hr = GetRegInt("DefaultProps\\Light","BorderColor", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_bordercolor = iTmp;
   else
      m_d.m_bordercolor = RGB(0,0,0);

   hr = GetRegString("DefaultProps\\Light", "Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   hr = GetRegInt("DefaultProps\\Light","EnableLighting", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_EnableLighting = iTmp;
   else
      m_d.m_EnableLighting = fTrue;
   hr = GetRegInt("DefaultProps\\Light","EnableOffLighting", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_EnableOffLighting = iTmp;
   else
      m_d.m_EnableOffLighting = fTrue;
   hr = GetRegStringAsFloat("DefaultProps\\Light","FadeSpeed", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fadeSpeed = fTmp;
   else
      m_d.m_fadeSpeed = 0.2f;
   hr = GetRegInt("DefaultProps\\Light","Bulb", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_BulbLight = iTmp;
   else
       m_d.m_BulbLight = fFalse;

}

void Light::WriteRegDefaults()
{
   SetRegValueFloat("DefaultProps\\Light","Falloff", m_d.m_falloff);
   SetRegValue("DefaultProps\\Light","LightState",REG_DWORD,&m_d.m_state,4);
   SetRegValue("DefaultProps\\Light","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Light","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Light","Color",REG_DWORD,&m_d.m_color,4);
   SetRegValue("DefaultProps\\Light","OffImage", REG_SZ, &m_d.m_szOffImage,lstrlen(m_d.m_szOffImage));
   SetRegValue("DefaultProps\\Light","OnImage", REG_SZ, &m_d.m_szOnImage, lstrlen(m_d.m_szOnImage));
   SetRegValue("DefaultProps\\Light","DisplayImage", REG_DWORD, &m_d.m_fDisplayImage,4);
   SetRegValue("DefaultProps\\Light","BlinkPattern", REG_SZ, &m_rgblinkpattern, lstrlen(m_rgblinkpattern));
   SetRegValue("DefaultProps\\Light","BlinkInterval", REG_DWORD, &m_blinkinterval,4);
   SetRegValue("DefaultProps\\Light","BorderColor", REG_DWORD, &m_d.m_bordercolor,4);
   SetRegValue("DefaultProps\\Light","Surface", REG_SZ, &m_d.m_szSurface, lstrlen(m_d.m_szSurface));
   SetRegValue("DefaultProps\\Light","EnableLighting", REG_DWORD, &m_d.m_EnableLighting,4);
   SetRegValue("DefaultProps\\Light","EnableOffLighting", REG_DWORD, &m_d.m_EnableOffLighting,4);
   SetRegValueFloat("DefaultProps\\Light","FadeSpeed", m_d.m_fadeSpeed);
   SetRegValueFloat("DefaultProps\\Light","Intensity", m_d.m_intensity);
   SetRegValue("DefaultProps\\Light","Bulb", REG_DWORD, &m_d.m_BulbLight,4);
}

Texture *Light::GetDisplayTexture()
{
    switch (m_d.m_state)
    {
        case LightStateOff: return m_ptable->GetImage(m_d.m_szOffImage);
        case LightStateOn:  return m_ptable->GetImage(m_d.m_szOnImage);
        default:            return NULL;
    }
}

void Light::PreRender(Sur * const psur)
{
   psur->SetBorderColor(-1,false,0);
   psur->SetFillColor(m_ptable->RenderSolid() ? m_d.m_color : -1);
   psur->SetObject(this);

   Texture *ppi;

   // workaround for the old round light object
   // after loading m_roundLight is true if an pre VP10 table was loaded
   // init the round light to the new custom one
   if( m_roundLight )
   {
      InitShape();
      m_roundLight=false;
   }
   switch (m_d.m_shape)
   {
   default:
   case ShapeCustom:
       Vector<RenderVertex> vvertex;
       GetRgVertex(&vvertex);

       // Check if we should display the image in the editor.
       if (m_ptable->RenderSolid() && m_d.m_fDisplayImage && (ppi = GetDisplayTexture()))
       {
           ppi->EnsureHBitmap();
           if (ppi->m_hbmGDIVersion)
               // Draw the polygon with an image applied.
               psur->PolygonImage(vvertex, ppi->m_hbmGDIVersion,
                   m_ptable->m_left, m_ptable->m_top, m_ptable->m_right, m_ptable->m_bottom, ppi->m_width, ppi->m_height);
       }
       else
           psur->Polygon(vvertex);

       for (int i=0;i<vvertex.Size();i++)
           delete vvertex.ElementAt(i);

       break;
   }
}

void Light::Render(Sur * const psur)
{
   bool fDrawDragpoints = ( (m_selectstate != eNotSelected) || (g_pvp->m_fAlwaysDrawDragPoints) );		//>>> added by chris
   
   // if the item is selected then draw the dragpoints (or if we are always to draw dragpoints)
   if (!fDrawDragpoints)
   {
      // if any of the dragpoints of this object are selected then draw all the dragpoints
      for (int i=0; i<m_vdpoint.Size(); i++)
      {
         const CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);
         if (pdp->m_selectstate != eNotSelected)
         {
            fDrawDragpoints = true;
            break;
         }
      }
   }

   RenderOutline(psur);

   if ( (m_d.m_shape == ShapeCustom) && fDrawDragpoints )	//<<< modified by chris
   {
      for (int i=0; i<m_vdpoint.Size(); i++)
      {
         CComObject<DragPoint> *pdp;
         pdp = m_vdpoint.ElementAt(i);
         psur->SetFillColor(-1);
         psur->SetBorderColor(RGB(0,0,200),false,0);
         psur->SetObject(pdp);

         if (pdp->m_fDragging)
            psur->SetBorderColor(RGB(0,255,0),false,0);

         psur->Ellipse2(pdp->m_v.x, pdp->m_v.y, 8);
      }
   }
}

void Light::RenderOutline(Sur * const psur)
{
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetLineColor(RGB(0,0,0),false,0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(NULL);

   switch (m_d.m_shape)
   {
   case ShapeCircle:
   default: 
       {
          psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_falloff + m_d.m_borderwidth);
          break;
       }

   case ShapeCustom: 
       {
          Vector<RenderVertex> vvertex;
          GetRgVertex(&vvertex);
          if (m_selectstate != eNotSelected)  
          {
              psur->SetBorderColor(RGB(255,0,0),false,0);
              psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_falloff + m_d.m_borderwidth);
              psur->SetBorderColor(RGB(0,0,0),false,0);
          }
          psur->Polygon(vvertex);

          for (int i=0;i<vvertex.Size();i++)
             delete vvertex.ElementAt(i);

          psur->SetObject((ISelect *)&m_lightcenter);
          break;
       }
   }

   if (m_d.m_shape == ShapeCustom || g_pvp->m_fAlwaysDrawLightCenters)
   {
      psur->Line(m_d.m_vCenter.x - 10.0f, m_d.m_vCenter.y, m_d.m_vCenter.x + 10.0f, m_d.m_vCenter.y);
      psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - 10.0f, m_d.m_vCenter.x, m_d.m_vCenter.y + 10.0f);
   }
}

void Light::RenderBlueprint(Sur *psur)
{
   RenderOutline(psur);
}

void Light::GetTimers(Vector<HitTimer> * const pvht)
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

void Light::GetHitShapes(Vector<HitObject> * const pvho)
{
}

void Light::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   switch (m_d.m_shape)
   {
   case ShapeCircle:
   default: {
      HitObject * const pho = CreateCircularHitPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height, m_d.m_falloff, 32);
      pvho->AddElement(pho);

	  break;
   }

   case ShapeCustom: {
      Vector<RenderVertex> vvertex;
      GetRgVertex(&vvertex);

      const int cvertex = vvertex.Size();
      Vertex3Ds * const rgv3d = new Vertex3Ds[cvertex];

      for (int i=0; i<cvertex; i++)
      {
         rgv3d[i].x = vvertex.ElementAt(i)->x;
         rgv3d[i].y = vvertex.ElementAt(i)->y;
         rgv3d[i].z = height;
         delete vvertex.ElementAt(i);
      }

      Hit3DPoly * const ph3dp = new Hit3DPoly(rgv3d, cvertex);
      pvho->AddElement(ph3dp);
      
	  break;
   }
   }
}

void Light::FreeBuffers()
{
   if( customVBuffer )
   {
      customVBuffer->release();
      customVBuffer=0;
   }
   if( normalVBuffer )
   {
      normalVBuffer->release();
      normalVBuffer=0;
   }
   if( customMoverVBuffer )
   {
      customMoverVBuffer->release();
      customMoverVBuffer=0;
   }
   if( normalMoverVBuffer )
   {
      normalMoverVBuffer->release();
      normalMoverVBuffer=0;
   }
   if( m_pInsertShader )
   {
      delete( m_pInsertShader );
      m_pInsertShader=0;
   }
}

void Light::EndPlay()
{
   // ensure not locked just in case the player exits during a LS sequence
   m_fLockedByLS = false;			//>>> added by chris

   IEditable::EndPlay();
   FreeBuffers();
}

void Light::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

//static const WORD rgiLightStatic0[3] = {0,1,2};
static const WORD rgiLightStatic1[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

void Light::PostRenderStatic(RenderDevice* pd3dDevice)
{
    TRACE_FUNCTION();

    if (m_fBackglass && !GetPTable()->GetDecalsEnabled())
        return;

    if (m_realState == LightStateBlinking)
        UpdateBlinker(g_pplayer->m_time_msec);

    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);

    bool useLightmap = false;

    const bool isOn = (m_realState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : !!m_realState;
    const float height = m_surfaceHeight;

    const float r = (float)(m_d.m_color & 255) * (float)(1.0/255.0);
    const float g = (float)(m_d.m_color & 65280) * (float)(1.0/65280.0);
    const float b = (float)(m_d.m_color & 16711680) * (float)(1.0/16711680.0);

    Texture* pin = NULL;
    Material mtrl;
    ppin3d->DisableLightMap();
    if (!m_fBackglass)
    {
        float depthbias = -BASEDEPTHBIAS;
        pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, *((DWORD*)&depthbias));
    }
   pd3dDevice->SetVertexDeclaration(pd3dDevice->m_pVertexNormalTexelTexelDeclaration);
    ppin3d->EnableAlphaTestReference(1);        // don't alpha blend, but do honor transparent pixels
    Texture *offTexel=NULL;
    if ( !m_d.m_BulbLight )
    {
        if ((offTexel = m_ptable->GetImage(m_d.m_szOffImage)) != NULL)
        {
            m_pInsertShader->Core()->SetTechnique("BasicLightWithTexture");
            if ( offTexel->m_pdsBufferColorKey )
                m_pInsertShader->Core()->SetTexture("OffTexture",ppin3d->m_pd3dDevice->m_texMan.LoadTexture(offTexel->m_pdsBufferColorKey));
            else if (offTexel->m_pdsBuffer )
                m_pInsertShader->Core()->SetTexture("OffTexture",ppin3d->m_pd3dDevice->m_texMan.LoadTexture(offTexel->m_pdsBuffer));
        }
        else
        {
            m_pInsertShader->Core()->SetTechnique("BasicLightWithoutTexture");
        }
    }
    else
    {
        m_pInsertShader->Core()->SetTechnique("BulbLight");
    }
    UINT cPasses=0;
    D3DXVECTOR4 center(m_d.m_vCenter.x, m_d.m_vCenter.y, m_surfaceHeight+0.05f, 0.0f);
    D3DXVECTOR4 diffColor(r,g,b,1.0f);
    m_pInsertShader->Core()->SetVector("lightCenter", &center);
    m_pInsertShader->Core()->SetVector("diffuseMaterial", &diffColor);
    m_pInsertShader->Core()->SetFloat("maxRange",m_d.m_falloff);
    if ( isOn )
    {
       if (m_d.m_currentIntensity<m_d.m_intensity )
       {
          m_d.m_currentIntensity+=m_d.m_fadeSpeed;
          if(m_d.m_currentIntensity>m_d.m_intensity )
             m_d.m_currentIntensity=m_d.m_intensity;
       }
    }
    else
    {
       if (m_d.m_currentIntensity>0.0f )
       {
          m_d.m_currentIntensity-=m_d.m_fadeSpeed;
          if(m_d.m_currentIntensity<0.0f )
             m_d.m_currentIntensity=0.0f;
       }
    }
    m_pInsertShader->Core()->SetFloat("intensity",m_d.m_currentIntensity);
    m_pInsertShader->Core()->Begin(&cPasses,0);
    m_pInsertShader->Core()->BeginPass(0);

    pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLELIST, customMoverVBuffer, 0, customMoverVertexNum);

    m_pInsertShader->Core()->EndPass();
    m_pInsertShader->Core()->End();

    pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
    pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);

    pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
    pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
}

void Light::PrepareStaticCustom()
{
   const float height = m_surfaceHeight;
   const float inv_tablewidth = 1.0f/(m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f/(m_ptable->m_bottom - m_ptable->m_top);

   // prepare static custom object
   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);

   const int cvertex = vvertex.Size();
   std::vector<RenderVertex> rgv(cvertex);

   for (int i=0; i<cvertex; ++i)
   {
      const int p1 = (i == 0) ? (cvertex-1) : (i-1);
      const int p2 = (i < cvertex-1) ? (i+1) : 0;

      const Vertex2D v1 = *vvertex.ElementAt(p1);
      const Vertex2D v2 = *vvertex.ElementAt(p2);
      const Vertex2D vmiddle = *vvertex.ElementAt(i);

      const float A = v1.y - vmiddle.y;
      const float B = vmiddle.x - v1.x;
      const float D = v2.y - vmiddle.y;
      const float E = vmiddle.x - v2.x;

	  // Find intersection of the two edges meeting this points, but
      // shift those lines outwards along their normals

      // Shift line along the normal
      const float C = vmiddle.y*v1.x - vmiddle.x*v1.y;/* - sqrtf(A*A + B*B)*m_d.m_borderwidth;*/

      // Shift line along the normal
      const float F = vmiddle.y*v2.x - vmiddle.x*v2.y;/* + sqrtf(D*D + E*E)*m_d.m_borderwidth;*/

      const float inv_det = 1.0f/(A*E - B*D);

      rgv[i].x = (B*F-E*C)*inv_det;
      rgv[i].y = (C*D-A*F)*inv_det;
   }

   for (int i=0; i<cvertex; i++)
      delete vvertex.ElementAt(i);

   VectorVoid vpoly;
   for (int i=0; i<cvertex; i++)
      vpoly.AddElement((void *)i);

   Vector<Triangle> vtri;
   PolygonToTriangles(rgv, &vpoly, &vtri);
   staticCustomVertexNum = vtri.Size()*3;
   std::vector<Vertex3D> staticCustomVertex(staticCustomVertexNum);

   if ( customVBuffer==NULL )
   {
      DWORD vertexType = (!m_fBackglass) ? MY_D3DFVF_VERTEX : MY_D3DTRANSFORMED_VERTEX;
      g_pplayer->m_pin3d.m_pd3dDevice->CreateVertexBuffer( staticCustomVertexNum, 0, vertexType, &customVBuffer);
   }

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   int k=0;
   for (int t=0; t<vtri.Size(); t++,k+=3)
   {
      const Triangle * const ptri = vtri.ElementAt(t);

      const RenderVertex * const pv0 = &rgv[ptri->a];
      const RenderVertex * const pv1 = &rgv[ptri->c];
      const RenderVertex * const pv2 = &rgv[ptri->b];

      staticCustomVertex[k  ].x = pv0->x;   staticCustomVertex[k  ].y = pv0->y;   staticCustomVertex[k  ].z = height+0.05f;
      staticCustomVertex[k+1].x = pv1->x;   staticCustomVertex[k+1].y = pv1->y;   staticCustomVertex[k+1].z = height+0.05f;
      staticCustomVertex[k+2].x = pv2->x;   staticCustomVertex[k+2].y = pv2->y;   staticCustomVertex[k+2].z = height+0.05f;

      if (!m_fBackglass)
      {
         staticCustomVertex[k  ].nx = 0;    staticCustomVertex[k  ].ny = 0;    staticCustomVertex[k  ].nz = 1.0f;
         staticCustomVertex[k+1].nx = 0;    staticCustomVertex[k+1].ny = 0;    staticCustomVertex[k+1].nz = 1.0f;
         staticCustomVertex[k+2].nx = 0;    staticCustomVertex[k+2].ny = 0;    staticCustomVertex[k+2].nz = 1.0f;

         ppin3d->CalcShadowCoordinates(&staticCustomVertex[k],3);
      }
      else
      {
         SetDiffuse(&staticCustomVertex[k], 3, RGB_TO_BGR(m_d.m_bordercolor));
         SetHUDVertices(&staticCustomVertex[k], 3);
      }
	  delete vtri.ElementAt(t);
   }

   Vertex3D *buf;
   customVBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
   memcpy( buf, &staticCustomVertex[0], staticCustomVertexNum*sizeof(Vertex3D));
   customVBuffer->unlock();
}

void Light::PrepareMoversCustom()
{
   Vector<RenderVertex> vvertex;
   GetRgVertex(&vvertex);
   const int cvertex = vvertex.Size();

   VectorVoid vpoly;
   float maxdist = 0;
   for (int i=0; i<cvertex; i++)
   {
      vpoly.AddElement((void *)i);

      const float dx = vvertex.ElementAt(i)->x - m_d.m_vCenter.x;
      const float dy = vvertex.ElementAt(i)->y - m_d.m_vCenter.y;
      const float dist = dx*dx + dy*dy;
      if (dist > maxdist)
         maxdist = dist;
   }
   const float inv_maxdist = (maxdist > 0.0f) ? 0.5f/sqrtf(maxdist) : 0.0f;
   const float inv_tablewidth = 1.0f/(m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f/(m_ptable->m_bottom - m_ptable->m_top);

   Vector<Triangle> vtri;
   PolygonToTriangles(vvertex, &vpoly, &vtri);

   const float height = m_surfaceHeight;

   Vertex3D *customMoverVertex[2];

   customMoverVertexNum = vtri.Size()*3;
   customMoverVertex[0] = new Vertex3D[customMoverVertexNum];
   customMoverVertex[1] = new Vertex3D[customMoverVertexNum];

   if ( customMoverVBuffer==NULL )
   {
      DWORD vertexType = (!m_fBackglass) ? MY_D3DFVF_VERTEX : MY_D3DTRANSFORMED_VERTEX;
      g_pplayer->m_pin3d.m_pd3dDevice->CreateVertexBuffer( customMoverVertexNum*2, 0, vertexType, &customMoverVBuffer);
   }

   Material mtrl;

   const float r = (float)(m_d.m_color & 255) * (float)(1.0/255.0);
   const float g = (float)(m_d.m_color & 65280) * (float)(1.0/65280.0);
   const float b = (float)(m_d.m_color & 16711680) * (float)(1.0/16711680.0);

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   for(int i=0; i<2; i++)
   {
      Texture* pin = NULL;
      if(i == LightStateOff ) 
         pin = m_ptable->GetImage(m_d.m_szOffImage);
	  else
      if(i == LightStateOn ) 
         pin = m_ptable->GetImage(m_d.m_szOnImage);

      int k=0;
      for (int t=0;t<vtri.Size();t++,k+=3)
      {
         const Triangle * const ptri = vtri.ElementAt(t);

         const RenderVertex * const pv0 = vvertex.ElementAt(ptri->a);
         const RenderVertex * const pv1 = vvertex.ElementAt(ptri->c);
         const RenderVertex * const pv2 = vvertex.ElementAt(ptri->b);

         customMoverVertex[i][k  ].x = pv0->x;   customMoverVertex[i][k  ].y = pv0->y;   customMoverVertex[i][k  ].z = height+0.1f;
         customMoverVertex[i][k+1].x = pv1->x;   customMoverVertex[i][k+1].y = pv1->y;   customMoverVertex[i][k+1].z = height+0.1f;
         customMoverVertex[i][k+2].x = pv2->x;   customMoverVertex[i][k+2].y = pv2->y;   customMoverVertex[i][k+2].z = height+0.1f;

         if(!m_fBackglass)
         {
            customMoverVertex[i][k  ].nx = 0; customMoverVertex[i][k  ].ny = 0; customMoverVertex[i][k  ].nz = 1.0f;
            customMoverVertex[i][k+1].nx = 0; customMoverVertex[i][k+1].ny = 0; customMoverVertex[i][k+1].nz = 1.0f;
            customMoverVertex[i][k+2].nx = 0; customMoverVertex[i][k+2].ny = 0; customMoverVertex[i][k+2].nz = 1.0f;
         }

         for (int l=0;l<3;l++)
         {
            if (!m_fBackglass)
               ppin3d->CalcShadowCoordinates(&customMoverVertex[i][k+l],1);

            // Check if we are using a custom texture.
            if (pin != NULL)
            {
               customMoverVertex[i][k+l].tu = customMoverVertex[i][k+l].x * inv_tablewidth;
               customMoverVertex[i][k+l].tv = customMoverVertex[i][k+l].y * inv_tableheight;
            }
            else
            {
               // Set texture coordinates for default light.
               const float dx = customMoverVertex[i][k+l].x - m_d.m_vCenter.x;
               const float dy = customMoverVertex[i][k+l].y - m_d.m_vCenter.y;
               customMoverVertex[i][k+l].tu = 0.5f + dx * inv_maxdist;
               customMoverVertex[i][k+l].tv = 0.5f + dy * inv_maxdist;
            }
         }

         if (m_fBackglass)
            SetHUDVertices(&customMoverVertex[i][k], 3);
      }

      if(i == LightStateOff) 
      {
         // Check if the light has an "off" texture.
         if ( pin == NULL )
         {
            // Set the texture to a default.
            mtrl.setAmbient( 1.0f, r*0.3f, g*0.3f, b*0.3f );
            mtrl.setDiffuse( 1.0f, r*0.3f, g*0.3f, b*0.3f );
            mtrl.setEmissive(0.0f, 0.0f, 0.0f, 0.0f );
         }
         else
         {
            // Set the texture to the one defined in the editor.
            mtrl.setAmbient( 1.0f, 1.0f, 1.0f, 1.0f );
            mtrl.setDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
            mtrl.setEmissive(0.0f, 0.0f, 0.0f, 0.0f );
         }
      } 
      else //LightStateOn 
      {
         // Check if the light has an "on" texture.
         if ( pin == NULL )
         {
            // Set the texture to a default.
            mtrl.setAmbient( 1.0f, 0.0f, 0.0f, 0.0f );
            mtrl.setDiffuse( 1.0f, 0.0f, 0.0f, 0.0f );
            mtrl.setEmissive(0.0f, r, g, b );
         }
         else
         {
            // Set the texture to the one defined in the editor.
            mtrl.setAmbient( 1.0f, 1.0f, 1.0f, 1.0f );
            mtrl.setDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
            mtrl.setEmissive(0.0f, 0.0f, 0.0f, 0.0f );
         }
      }
      if (m_fBackglass && GetPTable()->GetDecalsEnabled())
         SetDiffuseFromMaterial(customMoverVertex[i], customMoverVertexNum, &mtrl);

   }//for(i=0;i<2...)


   Vertex3D *buf;
   customMoverVBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
   memcpy( buf, customMoverVertex[0], customMoverVertexNum*sizeof(Vertex3D));
   memcpy( &buf[customMoverVertexNum], customMoverVertex[1], customMoverVertexNum*sizeof(Vertex3D));
   customMoverVBuffer->unlock();

   delete [] customMoverVertex[0];
   delete [] customMoverVertex[1];

   for (int i=0;i<cvertex;i++)
      delete vvertex.ElementAt(i);

   for (int i=0;i<vtri.Size();i++)
      delete vtri.ElementAt(i);
}

void Light::RenderSetup(RenderDevice* pd3dDevice)
{
    m_surfaceHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_zScale;

    if (m_realState == LightStateBlinking)
        RestartBlinker(g_pplayer->m_time_msec);

    const bool isOn = (m_realState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : !!m_realState;
    if( isOn )
       m_d.m_currentIntensity = m_d.m_intensity;
    else
       m_d.m_currentIntensity = 0.0f;

    if( m_pInsertShader==NULL )
    {
      m_pInsertShader = new Shader(g_pplayer->m_pin3d.m_pd3dDevice);
//      m_pInsertShader->Load("c:\\projects\\vp\\shader\\LightInsert.fx",true );
      m_pInsertShader->Load("LightInsert.fx",false );
      D3DMATRIX worldMat;
      D3DMATRIX viewMat;
      D3DMATRIX projMat;
      g_pplayer->m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_WORLD, &worldMat );
      g_pplayer->m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
      g_pplayer->m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_PROJECTION, &projMat);

      D3DXMATRIX matProj(projMat);
      D3DXMATRIX matView(viewMat);
      D3DXMATRIX matWorld(worldMat);
      D3DXMATRIX worldViewProj = matWorld * matView * matProj;
      m_pInsertShader->Core()->SetMatrix("matWorldViewProj", &worldViewProj);
      m_pInsertShader->Core()->SetMatrix("matWorld",  &matView);
      D3DXVECTOR4 cam( worldViewProj._41, worldViewProj._42, worldViewProj._43, 0 );
      m_pInsertShader->Core()->SetVector("camera", &cam);
    }

    m_d.m_currentIntensity = 0.0f;
    PrepareStaticCustom();
    PrepareMoversCustom();
}

void Light::RenderStatic(RenderDevice* pd3dDevice)
{
    if (m_d.m_borderwidth > 0)
    {
        const float height = m_surfaceHeight;
        Pin3D * const ppin3d = &g_pplayer->m_pin3d;
        if (!m_fBackglass)
            ppin3d->EnableLightMap(height);

        Material mtrl;
        mtrl.setColor( 1.0f, m_d.m_bordercolor );

        pd3dDevice->SetMaterial(mtrl);

        if((!m_fBackglass) || GetPTable()->GetDecalsEnabled())
        {
            if(m_d.m_shape == ShapeCustom)
                pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLELIST, customVBuffer, 0, staticCustomVertexNum);
            else
                pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, normalVBuffer, 0, 32 );
        }

        ppin3d->DisableLightMap();
    }
}

void Light::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Light::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   for (int i=0; i<m_vdpoint.Size(); i++)
   {
      CComObject<DragPoint> * const pdp = m_vdpoint.ElementAt(i);

      pdp->m_v.x += dx;
      pdp->m_v.y += dy;
   }

   m_ptable->SetDirtyDraw();
}

HRESULT Light::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(RADI), m_d.m_falloff);
   bw.WriteInt(FID(STAT), m_d.m_state);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteBool(FID(DISP), m_d.m_fDisplayImage);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(BPAT), m_rgblinkpattern);
   bw.WriteString(FID(IMG1), m_d.m_szOffImage);
   bw.WriteString(FID(IMG2), m_d.m_szOnImage);
   bw.WriteInt(FID(BINT), m_blinkinterval);
   bw.WriteInt(FID(BCOL), m_d.m_bordercolor);
   bw.WriteFloat(FID(BWTH), m_d.m_intensity);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteBool(FID(BGLS), m_fBackglass);
   bw.WriteBool(FID(ENLI), m_d.m_EnableLighting);
   bw.WriteBool(FID(ENOL), m_d.m_EnableOffLighting);
   bw.WriteFloat(FID(LIDB), m_d.m_depthBias);
   bw.WriteFloat(FID(FASP), m_d.m_fadeSpeed);
   bw.WriteBool(FID(BULT), m_d.m_BulbLight);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   //bw.WriteTag(FID(PNTS));
   HRESULT hr;
   if(FAILED(hr = SavePointData(pstm, hcrypthash, hcryptkey)))
      return hr;

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Light::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   m_d.m_falloff = 50;
   m_d.m_state = LightStateOff;
   m_d.m_shape = ShapeCustom;

   m_d.m_tdr.m_fTimerEnabled = fFalse;
   m_d.m_tdr.m_TimerInterval = 100;

   m_d.m_color = RGB(255,255,0);

   strcpy_s(m_rgblinkpattern, sizeof(m_rgblinkpattern), "10");
   m_blinkinterval = 125;
   m_d.m_borderwidth = 0;
   m_d.m_bordercolor = RGB(0,0,0);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   m_fLockedByLS = false;			//>>> added by chris
   m_realState	= m_d.m_state;		//>>> added by chris

   m_d.m_EnableLighting = fTrue;
   m_d.m_EnableOffLighting = fTrue;

   br.Load();
   return S_OK;
}

BOOL Light::LoadToken(int id, BiffReader *pbr)
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
      pbr->GetFloat(&m_d.m_falloff);
   }
   else if (id == FID(STAT))
   {
      pbr->GetInt(&m_d.m_state);
      m_realState	= m_d.m_state;		//>>> added by chris
   }
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_d.m_color);
   }
   else if (id == FID(IMG1))
   {
      pbr->GetString(m_d.m_szOffImage);
   }
   else if (id == FID(IMG2))
   {
      pbr->GetString(m_d.m_szOnImage);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(DISP))
   {
      pbr->GetBool(&m_d.m_fDisplayImage);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(SHAP))
   {
      m_roundLight=true;
   }
   else if (id == FID(BPAT))
   {
      pbr->GetString(m_rgblinkpattern);
   }
   else if (id == FID(BINT))
   {
      pbr->GetInt(&m_blinkinterval);
   }
   else if (id == FID(BCOL))
   {
      pbr->GetInt(&m_d.m_bordercolor);
   }
   else if (id == FID(BWTH))
   {
      pbr->GetFloat(&m_d.m_intensity);
   }
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(BGLS))
   {
      pbr->GetBool(&m_fBackglass);
   }
   if (id == FID(ENLI))
   {
      pbr->GetBool(&m_d.m_EnableLighting);
   }
   else if (id == FID(ENOL))
   {
      pbr->GetBool(&m_d.m_EnableOffLighting);
   }
   else if (id == FID(LIDB))
   {
      pbr->GetFloat(&m_d.m_depthBias);
   }
   else if (id == FID(FASP))
   {
       pbr->GetFloat(&m_d.m_fadeSpeed);
   }
   else if (id == FID(BULT))
   {
       pbr->GetBool(&m_d.m_BulbLight);
   }
   else
   {
      LoadPointToken(id, pbr, pbr->m_version);
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

HRESULT Light::InitPostLoad()
{
   return S_OK;
}

void Light::GetPointCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_vCenter;
}

void Light::PutPointCenter(const Vertex2D * const pv)
{
   m_d.m_vCenter = *pv;

   SetDirtyDraw();
}

void Light::EditMenu(HMENU hmenu)
{
   EnableMenuItem(hmenu, ID_WALLMENU_FLIP, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   EnableMenuItem(hmenu, ID_WALLMENU_MIRROR, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   EnableMenuItem(hmenu, ID_WALLMENU_ROTATE, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   EnableMenuItem(hmenu, ID_WALLMENU_SCALE, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
   EnableMenuItem(hmenu, ID_WALLMENU_ADDPOINT, MF_BYCOMMAND | ((m_d.m_shape != ShapeCustom) ? MF_GRAYED : MF_ENABLED));
}

void Light::DoCommand(int icmd, int x, int y)
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

STDMETHODIMP Light::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_ILight,
   };

   for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Light::get_Falloff(float *pVal)
{
   *pVal = m_d.m_falloff;

   return S_OK;
}

STDMETHODIMP Light::put_Falloff(float newVal)
{
   if (newVal < 0)
   {
      return E_FAIL;
   }

   STARTUNDO

   m_d.m_falloff = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Light::get_State(LightState *pVal)
{
   *pVal = m_d.m_state;

   return S_OK;
}

STDMETHODIMP Light::put_State(LightState newVal)
{
   STARTUNDO
   // if the light is locked by the LS then just change the state and don't change the actual light
   if (!m_fLockedByLS)
      setLightState(newVal);
   m_d.m_state = newVal;

   STOPUNDO

   return S_OK;
}

void Light::FlipY(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointY(pvCenter);
}

void Light::FlipX(Vertex2D * const pvCenter)
{
   IHaveDragPoints::FlipPointX(pvCenter);
}

void Light::Rotate(float ang, Vertex2D *pvCenter)
{
   IHaveDragPoints::RotatePoints(ang, pvCenter);
}

void Light::Scale(float scalex, float scaley, Vertex2D *pvCenter)
{
   IHaveDragPoints::ScalePoints(scalex, scaley, pvCenter);
}

void Light::Translate(Vertex2D *pvOffset)
{
   IHaveDragPoints::TranslatePoints(pvOffset);
}

STDMETHODIMP Light::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Light::put_Color(OLE_COLOR newVal)
{
   STARTUNDO

   m_d.m_color = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;

   return S_OK;
}

STDMETHODIMP Light::put_X(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.x = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Light::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Light::put_Y(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.y = newVal;

   STOPUNDO

      return S_OK;
}

void Light::InitShape()
{
   if ( m_vdpoint.Size() == 0 )
   {
      // First time shape has been set to custom - set up some points
      const float x = m_d.m_vCenter.x;
      const float y = m_d.m_vCenter.y;

      CComObject<DragPoint> *pdp;

      for( int i=8;i>0;i-- )
      {
         const float angle = (float)(M_PI*2.0/8.0)*(float)i;
         float xx = x + sinf(angle)*m_d.m_falloff;
         float yy = y - cosf(angle)*m_d.m_falloff;
         CComObject<DragPoint>::CreateInstance(&pdp);
         if (pdp)
         {
            pdp->AddRef();
            pdp->Init(this, xx, yy);
            pdp->m_fSmooth = TRUE;
            m_vdpoint.AddElement(pdp);
         }

      }

   }
}

STDMETHODIMP Light::get_BlinkPattern(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_rgblinkpattern, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Light::put_BlinkPattern(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_rgblinkpattern, 32, NULL, NULL);

   if (m_rgblinkpattern[0] == '\0')
   {
      m_rgblinkpattern[0] = '0';
      m_rgblinkpattern[1] = '\0';
   }

   if (g_pplayer)
   {
       RestartBlinker(g_pplayer->m_time_msec);
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_BlinkInterval(long *pVal)
{
   *pVal = m_blinkinterval;

   return S_OK;
}

STDMETHODIMP Light::put_BlinkInterval(long newVal)
{
   STARTUNDO

   m_blinkinterval = newVal;

   if (g_pplayer)
      m_timenextblink = g_pplayer->m_time_msec + m_blinkinterval;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_BorderColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_bordercolor;

   return S_OK;
}

STDMETHODIMP Light::put_BorderColor(OLE_COLOR newVal)
{
   STARTUNDO

   m_d.m_bordercolor = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_Intensity(float *pVal)
{
   *pVal = m_d.m_intensity;

   return S_OK;
}

STDMETHODIMP Light::put_Intensity(float newVal)
{
   STARTUNDO

   m_d.m_intensity = max(0.f, newVal);
   const bool isOn = (m_realState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : !!m_realState;
   if( isOn )
      m_d.m_currentIntensity = m_d.m_intensity;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_Surface(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Light::put_Surface(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}


STDMETHODIMP Light::get_OffImage(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szOffImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Light::put_OffImage(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szOffImage, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}



STDMETHODIMP Light::get_OnImage(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szOnImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Light::put_OnImage(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szOnImage, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_DisplayImage(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fDisplayImage);

   return S_OK;
}

STDMETHODIMP Light::put_DisplayImage(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fDisplayImage = VBTOF(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_EnableLighting(int *pVal)
{
   *pVal = m_d.m_EnableLighting;

   return S_OK;
}

STDMETHODIMP Light::put_EnableLighting(int newVal)
{
   STARTUNDO

   m_d.m_EnableLighting = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_EnableOffLighting(int *pVal)
{
   *pVal = m_d.m_EnableOffLighting;

   return S_OK;
}

STDMETHODIMP Light::put_EnableOffLighting(int newVal)
{
   STARTUNDO

      m_d.m_EnableOffLighting = newVal;

   STOPUNDO

      return S_OK;
}


STDMETHODIMP Light::get_DepthBias(float *pVal)
{
   *pVal = m_d.m_depthBias;

   return S_OK;
}

STDMETHODIMP Light::put_DepthBias(float newVal)
{
   STARTUNDO

   m_d.m_depthBias = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_FadeSpeed(float *pVal)
{
   *pVal = m_d.m_fadeSpeed;

   return S_OK;
}

STDMETHODIMP Light::put_FadeSpeed(float newVal)
{
   STARTUNDO

      m_d.m_fadeSpeed = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Light::get_Bulb(int *pVal)
{
    *pVal = m_d.m_BulbLight;

    return S_OK;
}

STDMETHODIMP Light::put_Bulb(int newVal)
{
    STARTUNDO

        m_d.m_BulbLight = newVal;

    STOPUNDO

    return S_OK;
}

void Light::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPLIGHT_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPLIGHT_STATE, IDS_STATE);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

void Light::lockLight()
{
   m_fLockedByLS = true;
}

void Light::unLockLight()
{
   m_fLockedByLS = false;
}

void Light::setLightStateBypass(const LightState newVal)
{
   lockLight();
   setLightState(newVal);
}

void Light::setLightState(const LightState newVal)
{
   if (newVal != m_realState) //state changed???
   {
      const LightState lastState = m_realState;
      m_realState = newVal;

      if (g_pplayer)
      {
         if (m_realState == LightStateBlinking)
         {
            m_timenextblink = g_pplayer->m_time_msec; // Start pattern right away // + m_d.m_blinkinterval;
            m_iblinkframe = 0; // reset pattern
         }
      }
   }
}
