#include "StdAfx.h"
#include "meshes/bulbLightMesh.h"
#include "meshes/bulbSocketMesh.h"

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


Light::Light() : m_lightcenter(this)
{
   m_menuid = IDR_SURFACEMENU;
   customMoverVBuffer = NULL;
   bulbLightIndexBuffer = NULL;
   bulbLightVBuffer = NULL;
   bulbSocketIndexBuffer = NULL;
   bulbSocketVBuffer = NULL;
   m_d.m_szOffImage[0] = 0;
   m_d.m_depthBias = 0.0f;
   m_d.m_shape = ShapeCustom;
   m_roundLight = false;
   m_propVisual = NULL;
}

Light::~Light()
{
   if( customMoverVBuffer )
   {
      customMoverVBuffer->release();
      customMoverVBuffer=0;
   }
   if( bulbLightIndexBuffer )
   {
       bulbLightIndexBuffer->release();
       bulbLightIndexBuffer=0;
   }
   if( bulbLightVBuffer )
   {
       bulbLightVBuffer->release();
       bulbLightVBuffer=0;
   }
   if( bulbSocketIndexBuffer )
   {
       bulbSocketIndexBuffer->release();
       bulbSocketIndexBuffer=0;
   }
   if( bulbSocketVBuffer )
   {
       bulbSocketVBuffer->release();
       bulbSocketVBuffer=0;
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
      m_d.m_falloff = 50.f;

   hr = GetRegStringAsFloat("DefaultProps\\Light","FalloffPower", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_falloff_power = fTmp;
   else
      m_d.m_falloff_power = 2.0f;

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

   hr = GetRegInt("DefaultProps\\Light","ColorFull", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_color2 = iTmp;
   else
      m_d.m_color2 = RGB(255,255,255);

   hr = GetRegString("DefaultProps\\Light","OffImage", m_d.m_szOffImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szOffImage[0] = 0;

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

   m_d.m_intensity_scale = 1.0f;

   /*hr = GetRegInt("DefaultProps\\Light","BorderColor", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_bordercolor = iTmp;
   else
      m_d.m_bordercolor = RGB(0,0,0);*/

   hr = GetRegString("DefaultProps\\Light", "Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Light","FadeSpeedUp", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fadeSpeedUp = fTmp;
   else
      m_d.m_fadeSpeedUp = 0.2f;

   hr = GetRegStringAsFloat("DefaultProps\\Light","FadeSpeedDown", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fadeSpeedDown = fTmp;
   else
      m_d.m_fadeSpeedDown = 0.2f;

   hr = GetRegInt("DefaultProps\\Light","Bulb", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_BulbLight = iTmp ? true : false;
   else
       m_d.m_BulbLight = false;

   hr = GetRegInt("DefaultProps\\Light","ImageMode", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_imageMode = iTmp ? true : false;
   else
       m_d.m_imageMode = false;

   hr = GetRegInt("DefaultProps\\Light","ShowBulbMesh", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_showBulbMesh = iTmp ? true : false;
   else
       m_d.m_showBulbMesh = false;

   hr = GetRegInt("DefaultProps\\Light","ShowReflectionOnBall", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_showReflectionOnBall = iTmp ? true : false;
   else
       m_d.m_showReflectionOnBall = true;

   hr = GetRegStringAsFloat("DefaultProps\\Light","ScaleBulbMesh", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_meshRadius = fTmp;
   else
       m_d.m_meshRadius = 20.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Light","BulbModulateVsAdd", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_modulate_vs_add = fTmp;
   else
       m_d.m_modulate_vs_add = 0.9f;

   hr = GetRegStringAsFloat("DefaultProps\\Light","BulbHaloHeight", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_bulbHaloHeight = fTmp;
   else
      m_d.m_bulbHaloHeight = 28.0f;
}

void Light::WriteRegDefaults()
{
   SetRegValueFloat("DefaultProps\\Light","Falloff", m_d.m_falloff);
   SetRegValueFloat("DefaultProps\\Light","FalloffPower", m_d.m_falloff_power);
   SetRegValue("DefaultProps\\Light","LightState",REG_DWORD,&m_d.m_state,4);
   SetRegValue("DefaultProps\\Light","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Light","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Light","Color",REG_DWORD,&m_d.m_color,4);
   SetRegValue("DefaultProps\\Light","ColorFull",REG_DWORD,&m_d.m_color2,4);
   SetRegValue("DefaultProps\\Light","OffImage", REG_SZ, &m_d.m_szOffImage,lstrlen(m_d.m_szOffImage));
   SetRegValue("DefaultProps\\Light","BlinkPattern", REG_SZ, &m_rgblinkpattern, lstrlen(m_rgblinkpattern));
   SetRegValue("DefaultProps\\Light","BlinkInterval", REG_DWORD, &m_blinkinterval,4);
   //SetRegValue("DefaultProps\\Light","BorderColor", REG_DWORD, &m_d.m_bordercolor,4);
   SetRegValue("DefaultProps\\Light","Surface", REG_SZ, &m_d.m_szSurface, lstrlen(m_d.m_szSurface));
   SetRegValueFloat("DefaultProps\\Light","FadeSpeedUp", m_d.m_fadeSpeedUp);
   SetRegValueFloat("DefaultProps\\Light","FadeSpeedDown", m_d.m_fadeSpeedDown);
   SetRegValueFloat("DefaultProps\\Light","Intensity", m_d.m_intensity);
   SetRegValueBool("DefaultProps\\Light","Bulb", m_d.m_BulbLight);
   SetRegValueBool("DefaultProps\\Light","ImageMode", m_d.m_imageMode);
   SetRegValueBool("DefaultProps\\Light","ShowBulbMesh", m_d.m_showBulbMesh);
   SetRegValueBool("DefaultProps\\Light","ShowReflectionOnBall", m_d.m_showReflectionOnBall);
   SetRegValueFloat("DefaultProps\\Light","ScaleBulbMesh", m_d.m_meshRadius);
   SetRegValueFloat("DefaultProps\\Light","BulbModulateVsAdd", m_d.m_modulate_vs_add);
   SetRegValueFloat("DefaultProps\\Light","BulbHaloHeight", m_d.m_bulbHaloHeight);
}

Texture *Light::GetDisplayTexture()
{
    switch (m_d.m_state)
    {
        case LightStateOff: return m_ptable->GetImage(m_d.m_szOffImage);
        default:            return NULL;
    }
}

void Light::PreRender(Sur * const psur)
{
   psur->SetBorderColor(-1,false,0);
   psur->SetFillColor(m_ptable->RenderSolid() ? (((m_d.m_color&0xFEFEFE)+(m_d.m_color2&0xFEFEFE))/2) : -1);
   psur->SetObject(this);

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
       std::vector<RenderVertex> vvertex;
       GetRgVertex(vvertex);

       // Check if we should display the image in the editor.
       psur->Polygon(vvertex);

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
          psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_falloff /*+ m_d.m_borderwidth*/);
          break;
       }

   case ShapeCustom: 
       {
          std::vector<RenderVertex> vvertex;
          GetRgVertex(vvertex);
          if (m_selectstate != eNotSelected)  
          {
              psur->SetBorderColor(RGB(255,0,0),false,0);
              psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_falloff /*+ m_d.m_borderwidth*/);
              psur->SetBorderColor(RGB(0,0,0),false,0);
          }
          psur->Polygon(vvertex);

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
      std::vector<RenderVertex> vvertex;
      GetRgVertex(vvertex);

      const int cvertex = vvertex.size();
      Vertex3Ds * const rgv3d = new Vertex3Ds[cvertex];

      for (int i=0; i<cvertex; i++)
      {
         rgv3d[i].x = vvertex[i].x;
         rgv3d[i].y = vvertex[i].y;
         rgv3d[i].z = height;
      }

      Hit3DPoly * const ph3dp = new Hit3DPoly(rgv3d, cvertex);
      pvho->AddElement(ph3dp);
      
	  break;
   }
   }
}

void Light::FreeBuffers()
{
   if( customMoverVBuffer )
   {
      customMoverVBuffer->release();
      customMoverVBuffer=0;
   }
   if( bulbLightIndexBuffer )
   {
       bulbLightIndexBuffer->release();
       bulbLightIndexBuffer=0;
   }
   if( bulbLightVBuffer )
   {
       bulbLightVBuffer->release();
       bulbLightVBuffer=0;
   }
   if( bulbSocketIndexBuffer )
   {
       bulbSocketIndexBuffer->release();
       bulbSocketIndexBuffer=0;
   }
   if( bulbSocketVBuffer )
   {
       bulbSocketVBuffer->release();
       bulbSocketVBuffer=0;
   }
}

void Light::EndPlay()
{
   // ensure not locked just in case the player exits during a LS sequence
   m_fLockedByLS = false;			//>>> added by chris

   IEditable::EndPlay();
   FreeBuffers();
}

float Light::GetDepth(const Vertex3Ds& viewDir) 
{ 
   return m_d.m_depthBias + viewDir.x * m_d.m_vCenter.x + viewDir.y * m_d.m_vCenter.y + viewDir.z * m_surfaceHeight; 
}

void Light::ClearForOverwrite()
{
   ClearPointsForOverwrite();
}

void Light::RenderBulbMesh(RenderDevice *pd3dDevice, COLORREF color, bool isOn)
{
    pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexNormalTexelDeclaration );
    pd3dDevice->basicShader->SetTechnique("basic_without_texture");
    Material mat;
    mat.m_cBase = 0x181818;
    mat.m_fWrapLighting = 0.5f;
    mat.m_bOpacityActive = false;
    mat.m_fOpacity = 1.0f;
    mat.m_cGlossy = 0xB4B4B4;
    mat.m_bIsMetal = false;
    mat.m_fEdge = 1.0f;
    mat.m_fRoughness = 0.9f;
    mat.m_cClearcoat = 0;
    pd3dDevice->basicShader->SetMaterial(&mat);

    pd3dDevice->basicShader->Begin(0);
    pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, bulbSocketVBuffer, 0, bulbSocketNumVertices, bulbSocketIndexBuffer, 0, bulbSocketNumFaces );
    pd3dDevice->basicShader->End();

    mat.m_cBase = 0;
    mat.m_fWrapLighting = 0.5f;
    mat.m_bOpacityActive = true;
    mat.m_fOpacity = 0.2f;
    mat.m_cGlossy = 0xFFFFFF;
    mat.m_bIsMetal = false;
    mat.m_fEdge = 1.0f;    
    mat.m_fRoughness = 0.9f;
    mat.m_cClearcoat = 0xFFFFFF;
    pd3dDevice->basicShader->SetMaterial(&mat);

    pd3dDevice->basicShader->Begin(0);
    pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, bulbLightVBuffer, 0, bulbLightNumVertices, bulbLightIndexBuffer, 0, bulbLightNumFaces );
    pd3dDevice->basicShader->End();
}

void Light::PostRenderStatic(RenderDevice* pd3dDevice)
{
    TRACE_FUNCTION();

	if(customMoverVBuffer == NULL) // in case of degenerate light
		return;

    if (m_fBackglass && !GetPTable()->GetDecalsEnabled())
        return;

	const U32 old_time_msec = (m_d.m_time_msec < g_pplayer->m_time_msec) ? m_d.m_time_msec : g_pplayer->m_time_msec;
    m_d.m_time_msec = g_pplayer->m_time_msec;
	const float diff_time_msec = (float)(g_pplayer->m_time_msec-old_time_msec);

    if (m_realState == LightStateBlinking)
        UpdateBlinker(g_pplayer->m_time_msec);

    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    const bool isOn = (m_realState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : !!m_realState;
    const float height = m_surfaceHeight;

    if (!m_fBackglass)
    {
	    pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
        float depthbias = -BASEDEPTHBIAS;
        pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, *((DWORD*)&depthbias));
    }

	//ppin3d->EnableAlphaTestReference(1);        // don't alpha blend, but do honor transparent pixels
	Vertex3D_NoTex2 centerHUD;
	centerHUD.x = m_d.m_vCenter.x;
	centerHUD.y = m_d.m_vCenter.y;
	centerHUD.z = 0.0f;
	if(m_fBackglass)
		SetHUDVertices(&centerHUD,1);
	D3DXVECTOR4 center(centerHUD.x, centerHUD.y, !m_fBackglass ? m_surfaceHeight+0.05f : 0.0f, 0.0f);
    D3DXVECTOR4 lightColor = convertColor(m_d.m_color);
    D3DXVECTOR4 lightColor2 = convertColor(m_d.m_color2);
    pd3dDevice->basicShader->Core()->SetVector("lightCenter", &center);
    pd3dDevice->basicShader->Core()->SetVector("lightColor", &lightColor);
    pd3dDevice->basicShader->Core()->SetVector("lightColor2", &lightColor2);
    pd3dDevice->basicShader->Core()->SetFloat("maxRange",m_d.m_falloff);
    pd3dDevice->basicShader->Core()->SetFloat("falloff_power",m_d.m_falloff_power);
    if ( isOn )
    {
       if (m_d.m_currentIntensity<m_d.m_intensity*m_d.m_intensity_scale )
       {
          m_d.m_currentIntensity+=m_d.m_fadeSpeedUp*diff_time_msec;
          if(m_d.m_currentIntensity>m_d.m_intensity*m_d.m_intensity_scale )
             m_d.m_currentIntensity=m_d.m_intensity*m_d.m_intensity_scale;
       }
    }
    else
    {
       if (m_d.m_currentIntensity>0.0f )
       {
          m_d.m_currentIntensity-=m_d.m_fadeSpeedDown*diff_time_msec;
          if(m_d.m_currentIntensity<0.0f )
             m_d.m_currentIntensity=0.0f;
       }
    }
    Texture *offTexel=NULL;
    if ( !m_d.m_BulbLight )
    {
        pd3dDevice->basicShader->Core()->SetBool("backglassMode",m_fBackglass);
        pd3dDevice->basicShader->Core()->SetBool("imageMode",m_d.m_imageMode);
        pd3dDevice->basicShader->SetMaterial(m_surfaceMaterial);

		if ((offTexel = m_ptable->GetImage(m_d.m_szOffImage)) != NULL)
        {
            pd3dDevice->basicShader->SetTechnique("light_with_texture");
            pd3dDevice->basicShader->SetTexture("Texture0", offTexel );
        }
        else
            pd3dDevice->basicShader->SetTechnique("light_without_texture");
    }
    else
	{		
        pd3dDevice->basicShader->SetTechnique("bulb_light");
	}

	pd3dDevice->SetVertexDeclaration(pd3dDevice->m_pVertexNormalTexelDeclaration);

    if ( m_d.m_showBulbMesh && m_d.m_BulbLight ) // blend bulb mesh hull additive over "normal" bulb to approximate the emission directly reaching the camera
    {
        pd3dDevice->basicShader->Core()->SetFloat("intensity",m_d.m_currentIntensity*0.02f); //!! make configurable?
        pd3dDevice->basicShader->Core()->SetFloat("blend_modulate_vs_add",0.00001f); // avoid 0, as it disables the blend

	    pd3dDevice->basicShader->Begin(0);
        pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, bulbLightVBuffer, 0, bulbLightNumVertices, bulbLightIndexBuffer, 0, bulbLightNumFaces );
		pd3dDevice->basicShader->End();
    }

    // render light shape
    if ( m_d.m_BulbLight )
        pd3dDevice->basicShader->Core()->SetFloat("blend_modulate_vs_add",max(m_d.m_modulate_vs_add,0.00001f)); // avoid 0, as it disables the blend
    pd3dDevice->basicShader->Core()->SetFloat("intensity",m_d.m_currentIntensity);
    pd3dDevice->basicShader->Begin(0);
    pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLELIST, customMoverVBuffer, 0, customMoverVertexNum);
    pd3dDevice->basicShader->End();
    
    pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);

	if (!m_fBackglass)
	{
	    pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
	    pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
	}
}

void Light::PrepareMoversCustom()
{
   std::vector<RenderVertex> vvertex;
   GetRgVertex(vvertex);
   const int cvertex = vvertex.size();

   if(cvertex == 0)
	   return;

   VectorVoid vpoly;
   float maxdist = 0;
   for (int i=0; i<cvertex; i++)
   {
      vpoly.AddElement((void *)i);

      const float dx = vvertex[i].x - m_d.m_vCenter.x;
      const float dy = vvertex[i].y - m_d.m_vCenter.y;
      const float dist = dx*dx + dy*dy;
      if (dist > maxdist)
         maxdist = dist;
   }
   const float inv_maxdist = (maxdist > 0.0f) ? 0.5f/sqrtf(maxdist) : 0.0f;
   const float inv_tablewidth = 1.0f/(m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f/(m_ptable->m_bottom - m_ptable->m_top);

   Vector<Triangle> vtri;
   PolygonToTriangles(vvertex, &vpoly, &vtri);

   float height = m_surfaceHeight;
   if ( m_d.m_BulbLight )
   {
       height += m_d.m_bulbHaloHeight*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
       m_surfaceHeight = height;
   }

   customMoverVertexNum = vtri.Size()*3;
   Vertex3D_NoTex2 *customMoverVertex = new Vertex3D_NoTex2[customMoverVertexNum];

   if ( customMoverVBuffer==NULL )
   {
      DWORD vertexType = (!m_fBackglass) ? MY_D3DFVF_NOTEX2_VERTEX : MY_D3DTRANSFORMED_NOTEX2_VERTEX;
      g_pplayer->m_pin3d.m_pd3dDevice->CreateVertexBuffer( customMoverVertexNum, 0, vertexType, &customMoverVBuffer);
   }

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   Texture* pin = m_ptable->GetImage(m_d.m_szOffImage);

   int k=0;
   for (int t=0;t<vtri.Size();t++,k+=3)
   {
	   const Triangle * const ptri = vtri.ElementAt(t);

	   const RenderVertex * const pv0 = &vvertex[ptri->a];
	   const RenderVertex * const pv1 = &vvertex[ptri->c];
	   const RenderVertex * const pv2 = &vvertex[ptri->b];

	   customMoverVertex[k  ].x = pv0->x;   customMoverVertex[k  ].y = pv0->y;   customMoverVertex[k  ].z = height+0.1f;
	   customMoverVertex[k+1].x = pv1->x;   customMoverVertex[k+1].y = pv1->y;   customMoverVertex[k+1].z = height+0.1f;
	   customMoverVertex[k+2].x = pv2->x;   customMoverVertex[k+2].y = pv2->y;   customMoverVertex[k+2].z = height+0.1f;

	   if(!m_fBackglass)
	   {
		   customMoverVertex[k  ].nx = 0; customMoverVertex[k  ].ny = 0; customMoverVertex[k  ].nz = 1.0f;
		   customMoverVertex[k+1].nx = 0; customMoverVertex[k+1].ny = 0; customMoverVertex[k+1].nz = 1.0f;
		   customMoverVertex[k+2].nx = 0; customMoverVertex[k+2].ny = 0; customMoverVertex[k+2].nz = 1.0f;
	   }

	   for (int l=0;l<3;l++)
	   {
		   // Check if we are using a custom texture.
		   if (pin != NULL)
		   {
			   customMoverVertex[k+l].tu = customMoverVertex[k+l].x * inv_tablewidth;
			   customMoverVertex[k+l].tv = customMoverVertex[k+l].y * inv_tableheight;
		   }
		   else
		   {
			   // Set texture coordinates for default light.
			   const float dx = customMoverVertex[k+l].x - m_d.m_vCenter.x;
			   const float dy = customMoverVertex[k+l].y - m_d.m_vCenter.y;
			   customMoverVertex[k+l].tu = 0.5f + dx * inv_maxdist;
			   customMoverVertex[k+l].tv = 0.5f + dy * inv_maxdist;
		   }
	   }

	   if (m_fBackglass)
		   SetHUDVertices(&customMoverVertex[k], 3);
   }

   Vertex3D_NoTex2 *buf;
   customMoverVBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);
   memcpy( buf, customMoverVertex, customMoverVertexNum*sizeof(Vertex3D_NoTex2));
   customMoverVBuffer->unlock();

   delete [] customMoverVertex;

   for (int i=0;i<vtri.Size();i++)
      delete vtri.ElementAt(i);
}

void Light::RenderSetup(RenderDevice* pd3dDevice)
{
    m_d.m_time_msec = g_pplayer->m_time_msec;

    m_surfaceHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
    m_surfaceMaterial = m_ptable->GetSurfaceMaterial(m_d.m_szSurface);

    if (m_realState == LightStateBlinking)
        RestartBlinker(g_pplayer->m_time_msec);

    const bool isOn = (m_realState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : !!m_realState;
    if( isOn )
       m_d.m_currentIntensity = m_d.m_intensity*m_d.m_intensity_scale;
    else
       m_d.m_currentIntensity = 0.0f;

    if ( m_d.m_BulbLight && m_d.m_showBulbMesh )
    {
        std::vector<WORD> indices(bulbLightNumFaces);
        Vertex3D_NoTex2 *buf;
        for( int i=0;i<bulbLightNumFaces;i++ ) indices[i] = bulbLightIndices[i];
        if (bulbLightIndexBuffer)
            bulbLightIndexBuffer->release();
        bulbLightIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer( indices );

        if (!bulbLightVBuffer)
            pd3dDevice->CreateVertexBuffer(bulbLightNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &bulbLightVBuffer);

        bulbLightVBuffer->lock(0, 0, (void**)&buf, 0);
        for( int i=0;i<bulbLightNumVertices;i++ )
        {
            buf[i].x = bulbLight[i].x*m_d.m_meshRadius+m_d.m_vCenter.x;
            buf[i].y = bulbLight[i].y*m_d.m_meshRadius+m_d.m_vCenter.y;
            buf[i].z = bulbLight[i].z*m_d.m_meshRadius*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_surfaceHeight;
            buf[i].nx = bulbLight[i].nx;
            buf[i].ny = bulbLight[i].ny;
            buf[i].nz = bulbLight[i].nz;
            buf[i].tu = bulbLight[i].tu;
            buf[i].tv = bulbLight[i].tv;
        }
        bulbLightVBuffer->unlock();
        indices.clear();
        indices.resize(bulbSocketNumFaces);
        for( int i=0;i<bulbSocketNumFaces;i++ ) indices[i] = bulbSocketIndices[i];
        if (bulbSocketIndexBuffer)
            bulbSocketIndexBuffer->release();
        bulbSocketIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer( indices );

        if (!bulbSocketVBuffer)
            pd3dDevice->CreateVertexBuffer(bulbSocketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &bulbSocketVBuffer);

        bulbSocketVBuffer->lock(0, 0, (void**)&buf, 0);
        for( int i=0;i<bulbSocketNumVertices;i++ )
        {
            buf[i].x = bulbSocket[i].x*m_d.m_meshRadius+m_d.m_vCenter.x;
            buf[i].y = bulbSocket[i].y*m_d.m_meshRadius+m_d.m_vCenter.y;
            buf[i].z = bulbSocket[i].z*m_d.m_meshRadius*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_surfaceHeight;
            buf[i].nx = bulbSocket[i].nx;
            buf[i].ny = bulbSocket[i].ny;
            buf[i].nz = bulbSocket[i].nz;
            buf[i].tu = bulbSocket[i].tu;
            buf[i].tv = bulbSocket[i].tv;
        }
        bulbSocketVBuffer->unlock();
    }

    PrepareMoversCustom();
}

void Light::RenderStatic(RenderDevice* pd3dDevice)
{
    if ( m_d.m_BulbLight && m_d.m_showBulbMesh)
        RenderBulbMesh(pd3dDevice,0,false);
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
   bw.WriteFloat(FID(FAPO), m_d.m_falloff_power);
   bw.WriteInt(FID(STAT), m_d.m_state);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteInt(FID(COL2), m_d.m_color2);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(BPAT), m_rgblinkpattern);
   bw.WriteString(FID(IMG1), m_d.m_szOffImage);
   bw.WriteInt(FID(BINT), m_blinkinterval);
   //bw.WriteInt(FID(BCOL), m_d.m_bordercolor);
   bw.WriteFloat(FID(BWTH), m_d.m_intensity);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteBool(FID(BGLS), m_fBackglass);
   bw.WriteFloat(FID(LIDB), m_d.m_depthBias);
   bw.WriteFloat(FID(FASP), m_d.m_fadeSpeedUp);
   bw.WriteFloat(FID(FASD), m_d.m_fadeSpeedDown);
   bw.WriteBool(FID(BULT), m_d.m_BulbLight);
   bw.WriteBool(FID(IMMO), m_d.m_imageMode);
   bw.WriteBool(FID(SHBM), m_d.m_showBulbMesh);
   bw.WriteBool(FID(SHRB), m_d.m_showReflectionOnBall);
   bw.WriteFloat(FID(BMSC), m_d.m_meshRadius);
   bw.WriteFloat(FID(BMVA), m_d.m_modulate_vs_add);
   bw.WriteFloat(FID(BHHI), m_d.m_bulbHaloHeight);

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

   m_d.m_falloff = 50.f;
   m_d.m_falloff_power = 2.0f;
   m_d.m_state = LightStateOff;
   m_d.m_shape = ShapeCustom;

   m_d.m_tdr.m_fTimerEnabled = fFalse;
   m_d.m_tdr.m_TimerInterval = 100;

   m_d.m_color = RGB(255,255,0);
   m_d.m_color2 = RGB(255,255,255);

   strcpy_s(m_rgblinkpattern, sizeof(m_rgblinkpattern), "10");
   m_blinkinterval = 125;
   //m_d.m_borderwidth = 0;
   //m_d.m_bordercolor = RGB(0,0,0);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   m_fLockedByLS = false;			//>>> added by chris
   m_realState	= m_d.m_state;		//>>> added by chris

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
   else if (id == FID(FAPO))
   {
      pbr->GetFloat(&m_d.m_falloff_power);
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
   else if (id == FID(COL2))
   {
      pbr->GetInt(&m_d.m_color2);
   }
   else if (id == FID(IMG1))
   {
      pbr->GetString(m_d.m_szOffImage);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
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
   /*else if (id == FID(BCOL))
   {
      pbr->GetInt(&m_d.m_bordercolor);
   }*/
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
   else if (id == FID(LIDB))
   {
      pbr->GetFloat(&m_d.m_depthBias);
   }
   else if (id == FID(FASP))
   {
       pbr->GetFloat(&m_d.m_fadeSpeedUp);
   }
   else if (id == FID(FASD))
   {
       pbr->GetFloat(&m_d.m_fadeSpeedDown);
   }
   else if (id == FID(BULT))
   {
       pbr->GetBool(&m_d.m_BulbLight);
   }
   else if (id == FID(IMMO))
   {
       pbr->GetBool(&m_d.m_imageMode);
   }
   else if (id == FID(SHBM))
   {
       pbr->GetBool(&m_d.m_showBulbMesh);
   }
   else if (id == FID(SHRB))
   {
       pbr->GetBool(&m_d.m_showReflectionOnBall);
   }
   else if (id == FID(BMSC))
   {
       pbr->GetFloat(&m_d.m_meshRadius);
   }
   else if (id == FID(BMVA))
   {
       pbr->GetFloat(&m_d.m_modulate_vs_add);
   }
   else if (id == FID(BHHI))
   {
      pbr->GetFloat(&m_d.m_bulbHaloHeight);
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

STDMETHODIMP Light::get_FalloffPower(float *pVal)
{
   *pVal = m_d.m_falloff_power;

   return S_OK;
}

STDMETHODIMP Light::put_FalloffPower(float newVal)
{
   STARTUNDO

   m_d.m_falloff_power = newVal;

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

STDMETHODIMP Light::get_ColorFull(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color2;

   return S_OK;
}

STDMETHODIMP Light::put_ColorFull(OLE_COLOR newVal)
{
   STARTUNDO

   m_d.m_color2 = newVal;

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
       RestartBlinker(g_pplayer->m_time_msec);

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
      m_d.m_currentIntensity = m_d.m_intensity*m_d.m_intensity_scale;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_IntensityScale(float *pVal)
{
   *pVal = m_d.m_intensity_scale;

   return S_OK;
}

STDMETHODIMP Light::put_IntensityScale(float newVal)
{
   STARTUNDO

   m_d.m_intensity_scale = newVal;
   const bool isOn = (m_realState == LightStateBlinking) ? (m_rgblinkpattern[m_iblinkframe] == '1') : !!m_realState;
   if( isOn )
      m_d.m_currentIntensity = m_d.m_intensity*m_d.m_intensity_scale;
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


STDMETHODIMP Light::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szOffImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Light::put_Image(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szOffImage, 32, NULL, NULL);

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

STDMETHODIMP Light::get_FadeSpeedUp(float *pVal)
{
   *pVal = m_d.m_fadeSpeedUp;

   return S_OK;
}

STDMETHODIMP Light::put_FadeSpeedUp(float newVal)
{
   STARTUNDO

   m_d.m_fadeSpeedUp = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_FadeSpeedDown(float *pVal)
{
   *pVal = m_d.m_fadeSpeedDown;

   return S_OK;
}

STDMETHODIMP Light::put_FadeSpeedDown(float newVal)
{
   STARTUNDO

   m_d.m_fadeSpeedDown = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Light::get_Bulb(VARIANT_BOOL *pVal)
{
    *pVal = m_d.m_BulbLight;

    return S_OK;
}

STDMETHODIMP Light::put_Bulb(VARIANT_BOOL newVal)
{
    STARTUNDO

    m_d.m_BulbLight = VBTOF(newVal);

    STOPUNDO

    return S_OK;
}

STDMETHODIMP Light::get_ImageMode(VARIANT_BOOL *pVal)
{
    *pVal = m_d.m_imageMode;

    return S_OK;
}

STDMETHODIMP Light::put_ImageMode(VARIANT_BOOL newVal)
{
    STARTUNDO

    m_d.m_imageMode = VBTOF(newVal);

    STOPUNDO

    return S_OK;
}

STDMETHODIMP Light::get_ShowBulbMesh(VARIANT_BOOL *pVal)
{
    *pVal = m_d.m_showBulbMesh;

    return S_OK;
}

STDMETHODIMP Light::put_ShowBulbMesh(VARIANT_BOOL newVal)
{
    STARTUNDO

    m_d.m_showBulbMesh = VBTOF(newVal);

    STOPUNDO

    return S_OK;
}

STDMETHODIMP Light::get_ShowReflectionOnBall(VARIANT_BOOL *pVal)
{
    *pVal = m_d.m_showReflectionOnBall;

    return S_OK;
}

STDMETHODIMP Light::put_ShowReflectionOnBall(VARIANT_BOOL newVal)
{
    STARTUNDO

    m_d.m_showReflectionOnBall = VBTOF(newVal);

    STOPUNDO

    return S_OK;
}

STDMETHODIMP Light::get_ScaleBulbMesh(float *pVal)
{
    *pVal = m_d.m_meshRadius;

    return S_OK;
}

STDMETHODIMP Light::put_ScaleBulbMesh(float newVal)
{
    STARTUNDO

    m_d.m_meshRadius = newVal;

    STOPUNDO

    return S_OK;
}

STDMETHODIMP Light::get_BulbModulateVsAdd(float *pVal)
{
    *pVal = m_d.m_modulate_vs_add;

    return S_OK;
}

STDMETHODIMP Light::put_BulbModulateVsAdd(float newVal)
{
    STARTUNDO

    m_d.m_modulate_vs_add = newVal;

    STOPUNDO

    return S_OK;
}

STDMETHODIMP Light::get_BulbHaloHeight(float *pVal)
{
   *pVal = m_d.m_bulbHaloHeight;

   return S_OK;
}

STDMETHODIMP Light::put_BulbHaloHeight(float newVal)
{
   STARTUNDO

      m_d.m_bulbHaloHeight = newVal;

   STOPUNDO

      return S_OK;
}

void Light::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   m_propVisual = new PropertyPane(IDD_PROPLIGHT_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(m_propVisual);

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

void Light::UpdatePropertyPanes()
{
    if ( m_propVisual==NULL )
        return;

    if ( !m_d.m_BulbLight )
    {
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_SHOW_BULB_MESH), FALSE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_SCALE_BULB_MESH), FALSE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_BULB_MODULATE_VS_ADD), FALSE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_HALO_EDIT), FALSE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_IMAGE_MODE), TRUE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,DISPID_Image), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_SHOW_BULB_MESH), TRUE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_SCALE_BULB_MESH), TRUE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_BULB_MODULATE_VS_ADD), TRUE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_HALO_EDIT), TRUE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,IDC_IMAGE_MODE), FALSE);
        EnableWindow(GetDlgItem(m_propVisual->dialogHwnd,DISPID_Image), FALSE);
    }
}
