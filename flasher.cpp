#include "StdAfx.h"

Flasher::Flasher()
{
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

   hr = GetRegStringAsFloat("DefaultProps\\Flasher","SizeX", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_sizeX = fTmp;
   else
      m_d.m_sizeX = 100.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flasher","SizeY", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_sizeY = fTmp;
   else
      m_d.m_sizeY = 100.0f;

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

   hr = GetRegString("DefaultProps\\Flasher","Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

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
       m_d.m_fDisplayTexture = fFalse;

}

void Flasher::WriteRegDefaults()
{
   char strTmp[40];

   sprintf_s(strTmp, 40, "%f", m_d.m_height);
   SetRegValue("DefaultProps\\Flasher","Height", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_sizeX);
   SetRegValue("DefaultProps\\Flasher","SizeX", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_sizeY);
   SetRegValue("DefaultProps\\Flasher","SizeY", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_rotX);
   SetRegValue("DefaultProps\\Flasher","RotX", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_rotY);
   SetRegValue("DefaultProps\\Flasher","RotY", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_rotZ);
   SetRegValue("DefaultProps\\Flasher","RotZ", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Flasher","Color",REG_DWORD,&m_d.m_color,4);
   SetRegValue("DefaultProps\\Flasher","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Flasher","TimerInterval",REG_DWORD,&m_d.m_tdr.m_TimerInterval,4);
   SetRegValue("DefaultProps\\Flasher","Image", REG_SZ, &m_d.m_szImage, strlen(m_d.m_szImage));
   SetRegValue("DefaultProps\\Flasher","Alpha",REG_DWORD,&m_d.m_fAlpha,4);
   SetRegValue("DefaultProps\\Flasher","Visible",REG_DWORD,&m_d.m_IsVisible,4);
   SetRegValue("DefaultProps\\Flasher","AddBlend",REG_DWORD,&m_d.m_fAddBlend,4);
   SetRegValue("DefaultProps\\Flasher","DisplayTexture", REG_DWORD, &m_d.m_fDisplayTexture,4);

}

void Flasher::PreRender(Sur * const psur)
{
   //make 1 wire ramps look unique in editor - uses ramp color
   psur->SetFillColor(m_d.m_color);
   psur->SetBorderColor(-1,false,0);
   psur->SetObject(this);

   const float halfwidth = m_d.m_sizeX * 0.5f;
   const float halfheight = m_d.m_sizeY * 0.5f;

   const float radangle = ANGTORAD(m_d.m_rotZ);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);
   float minx=FLT_MAX;
   float miny=FLT_MAX;
   float maxx=-FLT_MAX;
   float maxy=-FLT_MAX;

   const Vertex2D rgv[4] = 
   {
      Vertex2D(m_d.m_vCenter.x + sn*halfheight - cs*halfwidth,
      m_d.m_vCenter.y - cs*halfheight - sn*halfwidth),

      Vertex2D(m_d.m_vCenter.x + sn*halfheight + cs*halfwidth,
      m_d.m_vCenter.y - cs*halfheight + sn*halfwidth),

      Vertex2D(m_d.m_vCenter.x - sn*halfheight + cs*halfwidth,
      m_d.m_vCenter.y + cs*halfheight + sn*halfwidth),

      Vertex2D(m_d.m_vCenter.x - sn*halfheight - cs*halfwidth,
      m_d.m_vCenter.y + cs*halfheight - sn*halfwidth)
   };
   Texture *ppi;
   if (m_d.m_fDisplayTexture && (ppi = m_ptable->GetImage(m_d.m_szImage)))
   {
      for( int i=0;i<4;i++ )
      {
         if( rgv[i].x<minx) minx=rgv[i].x;
         if( rgv[i].x>maxx) maxx=rgv[i].x;
         if( rgv[i].y<miny) miny=rgv[i].y;
         if( rgv[i].y>maxy) maxy=rgv[i].y;
      }

      Vector<RenderVertex> verts;
      RenderVertex * const v1 = new RenderVertex;
      v1->x = rgv[0].x; v1->y = rgv[0].y; 
      verts.AddElement(v1);
      RenderVertex * const v2 = new RenderVertex;
      v2->x = rgv[1].x; v2->y = rgv[1].y; 
      verts.AddElement(v2);
      RenderVertex * const v3 = new RenderVertex;
      v3->x = rgv[2].x; v3->y = rgv[2].y; 
      verts.AddElement(v3);
      RenderVertex * const v4 = new RenderVertex;
      v4->x = rgv[3].x; v4->y = rgv[3].y; 
      verts.AddElement(v4);
      ppi->EnsureHBitmap();
      if (ppi->m_hbmGDIVersion)
         psur->PolygonImage(verts, ppi->m_hbmGDIVersion, minx, miny, minx+(maxx-minx), miny+(maxy-miny), ppi->m_width, ppi->m_height);
      //psur->PolygonImage(verts, ppi->m_hbmGDIVersion, m_ptable->m_left, m_ptable->m_top, m_ptable->m_right, m_ptable->m_bottom, ppi->m_width, ppi->m_height);
      else
      {
         // Do nothing for now to indicate to user that there is a problem
      }
      for( int i=0;i<verts.Size();i++) 
         delete(verts.ElementAt(i));
   }
   else
      psur->Polygon(rgv, 4);

}

void Flasher::Render(Sur * const psur)
{
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetLineColor(RGB(0,0,0),false,0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(NULL);

   const float halfwidth = m_d.m_sizeX * 0.5f;
   const float halfheight = m_d.m_sizeY * 0.5f;

   const float radangle = ANGTORAD(m_d.m_rotZ);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   const Vertex2D rgv[4] = 
   {
      Vertex2D(m_d.m_vCenter.x + sn*halfheight - cs*halfwidth,
      m_d.m_vCenter.y - cs*halfheight - sn*halfwidth),

      Vertex2D(m_d.m_vCenter.x + sn*halfheight + cs*halfwidth,
      m_d.m_vCenter.y - cs*halfheight + sn*halfwidth),

      Vertex2D(m_d.m_vCenter.x - sn*halfheight + cs*halfwidth,
      m_d.m_vCenter.y + cs*halfheight + sn*halfwidth),

      Vertex2D(m_d.m_vCenter.x - sn*halfheight - cs*halfwidth,
      m_d.m_vCenter.y + cs*halfheight - sn*halfwidth)
   };

      psur->Polygon(rgv, 4);

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
}

void Flasher::RenderSetup(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice = (RenderDevice*)_pd3dDevice;

   pd3dDevice->CreateVertexBuffer(4, 0, MY_D3DFVF_NOLIGHTING_VERTEX, &dynamicVertexBuffer);
   NumVideoBytes += 4*sizeof(Vertex3D_NoLighting);     
   solidMaterial.setColor( 1.0f, m_d.m_color );

   const float halfwidth = m_d.m_sizeX*0.5f;
   const float halfheight = m_d.m_sizeY*0.5f;
   const float height = m_d.m_height*m_ptable->m_zScale;

   vertices[0].x = m_d.m_vCenter.x - halfwidth;
   vertices[0].y = m_d.m_vCenter.y - halfheight;
   vertices[0].z = height;
   vertices[0].color = m_d.m_color;
   vertices[0].tu = 0;
   vertices[0].tv = 0;

   vertices[1].x = m_d.m_vCenter.x + halfwidth;
   vertices[1].y = m_d.m_vCenter.y - halfheight;
   vertices[1].z = height;
   vertices[1].color = m_d.m_color;
   vertices[1].tu = 1.0f;
   vertices[1].tv = 0;

   vertices[2].x = m_d.m_vCenter.x + halfwidth;
   vertices[2].y = m_d.m_vCenter.y + halfheight;
   vertices[2].z = height;
   vertices[2].color = m_d.m_color;
   vertices[2].tu = 1.0f;
   vertices[2].tv = 1.0f;

   vertices[3].x = m_d.m_vCenter.x - halfwidth;
   vertices[3].y = m_d.m_vCenter.y + halfheight;
   vertices[3].z = height;
   vertices[3].color = m_d.m_color;
   vertices[3].tu = 0;
   vertices[3].tv = 1.0f;
}

void Flasher::RenderStatic(const RenderDevice* _pd3dDevice)
{	
}

void Flasher::SetObjectPos()
{
   g_pvp->SetObjectPosCur(0, 0);
}

void Flasher::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   m_ptable->SetDirtyDraw();
}

HRESULT Flasher::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
   bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
   bw.WriteFloat(FID(FHEI), m_d.m_height);
   bw.WriteFloat(FID(FSIX), m_d.m_sizeX);
   bw.WriteFloat(FID(FSIY), m_d.m_sizeY);
   bw.WriteFloat(FID(FLAX), m_d.m_vCenter.x);
   bw.WriteFloat(FID(FLAY), m_d.m_vCenter.y);
   bw.WriteFloat(FID(FROX), m_d.m_rotX);
   bw.WriteFloat(FID(FROY), m_d.m_rotY);
   bw.WriteFloat(FID(FROZ), m_d.m_rotZ);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteInt(FID(FALP), m_d.m_fAlpha);
   bw.WriteBool(FID(FVIS), m_d.m_IsVisible);
   bw.WriteBool(FID(ADDB), m_d.m_fAddBlend);
   bw.WriteBool(FID(DSPT), m_d.m_fDisplayTexture);
   bw.WriteFloat(FID(FLDB), m_d.m_depthBias);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(ENDB));

   return S_OK;
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
   else if (id == FID(FSIX))
   {
      pbr->GetFloat(&m_d.m_sizeX);
   }
   else if (id == FID(FSIY))
   {
      pbr->GetFloat(&m_d.m_sizeY);
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
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(FALP))
   {
      int iTmp;
      pbr->GetInt(&iTmp);
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
   else
   {
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

STDMETHODIMP Flasher::get_SizeX(float *pVal)
{
   *pVal = m_d.m_sizeX;

   return S_OK;
}

STDMETHODIMP Flasher::put_SizeX(float newVal)
{
   if(m_d.m_sizeX != newVal)
   {
	   STARTUNDO

	   m_d.m_sizeX = newVal;
	   dynamicVertexBufferRegenerate = true;

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_SizeY(float *pVal)
{
   *pVal = m_d.m_sizeY;

   return S_OK;
}

STDMETHODIMP Flasher::put_SizeY(float newVal)
{
   if(m_d.m_sizeY != newVal)
   {
      STARTUNDO

         m_d.m_sizeY = newVal;
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


STDMETHODIMP Flasher::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flasher::put_Image(BSTR newVal)
{
   char m_szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_szImage, 32, NULL, NULL);

   if(strcmp(m_szImage,m_d.m_szImage) != 0)
   {
	   STARTUNDO

	   strcpy_s(m_d.m_szImage, MAXTOKEN, m_szImage);

	   STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Flasher::get_Alpha(long *pVal)
{
   *pVal = m_d.m_fAlpha;
   return S_OK;
}

STDMETHODIMP Flasher::put_Alpha(long newVal)
{
   STARTUNDO

   m_d.m_fAlpha = newVal;
   if (m_d.m_fAlpha>255 ) m_d.m_fAlpha=255;
   if (m_d.m_fAlpha<0 ) m_d.m_fAlpha=0;
   
   STOPUNDO

   return S_OK;
}


STDMETHODIMP Flasher::get_IsVisible(VARIANT_BOOL *pVal) //temporary value of object
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_IsVisible);

   return S_OK;
}

STDMETHODIMP Flasher::put_IsVisible(VARIANT_BOOL newVal)
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

STDMETHODIMP Flasher::get_UpdateRegions(VARIANT_BOOL *pVal)
{
   //!! deprecated
   *pVal = (VARIANT_BOOL)FTOVB(false);

   return S_OK;
}

STDMETHODIMP Flasher::put_UpdateRegions(VARIANT_BOOL newVal)
{
   //!! deprecated
   return S_OK;
}

STDMETHODIMP Flasher::TriggerSingleUpdate() 
{
   //!! deprecated
   return S_OK;
}

STDMETHODIMP Flasher::get_AddBlend(VARIANT_BOOL *pVal)
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

// Always called each frame to render over everything else (along with primitives)
// Same code as RenderStatic (with the exception of the alpha tests).
// Also has less drawing calls by bundling seperate calls.
void Flasher::PostRenderStatic(const RenderDevice* _pd3dDevice)
{
    TRACE_FUNCTION();
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   // Don't render if invisible.
   if(!m_d.m_IsVisible) 
      return;

   if ( m_d.m_sizeX==0.0f && m_d.m_sizeY==0.0f )
   {
      dynamicVertexBufferRegenerate=false;
      return;
   }

      Pin3D * const ppin3d = &g_pplayer->m_pin3d;
      Texture * const pin = m_ptable->GetImage(m_d.m_szImage);

      if (pin)
      {
         pin->CreateAlphaChannel();
         pin->Set(ePictureTexture);

		 ppin3d->SetTextureFilter( ePictureTexture, TEXTURE_MODE_TRILINEAR );
		 
		 pd3dDevice->SetMaterial(textureMaterial);
      }
      else
	  {
         ppin3d->SetTexture(NULL);
         solidMaterial.setColor(1.0f, m_d.m_color );
		 pd3dDevice->SetMaterial(solidMaterial);
	  }

      if(dynamicVertexBufferRegenerate)
      {
         dynamicVertexBufferRegenerate = false;

         Vertex3D_NoLighting *buf;
         dynamicVertexBuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY);

         const float halfwidth = m_d.m_sizeX*0.5f;
         const float halfheight = m_d.m_sizeY*0.5f;
         const float height = m_d.m_height*m_ptable->m_zScale;

         Vertex3D_NoLighting lvertices[4];
         lvertices[0].x = m_d.m_vCenter.x - halfwidth;
         lvertices[0].y = m_d.m_vCenter.y - halfheight;
         lvertices[0].z = height;
         lvertices[0].color = m_d.m_color;
         lvertices[0].tu = 0;
         lvertices[0].tv = 0;

         lvertices[1].x = m_d.m_vCenter.x + halfwidth;
         lvertices[1].y = m_d.m_vCenter.y - halfheight;
         lvertices[1].z = height;
         lvertices[1].color = m_d.m_color;
         lvertices[1].tu = 1.0f;
         lvertices[1].tv = 0;

         lvertices[2].x = m_d.m_vCenter.x + halfwidth;
         lvertices[2].y = m_d.m_vCenter.y + halfheight;
         lvertices[2].z = height;
         lvertices[2].color = m_d.m_color;
         lvertices[2].tu = 1.0f;
         lvertices[2].tv = 1.0f;

         lvertices[3].x = m_d.m_vCenter.x - halfwidth;
         lvertices[3].y = m_d.m_vCenter.y + halfheight;
         lvertices[3].z = height;
         lvertices[3].color = m_d.m_color;
         lvertices[3].tu = 0;
         lvertices[3].tv = 1.0f;

         Matrix3D tempMatrix,RTmatrix,TMatrix,T2Matrix;
         RTmatrix.SetIdentity();
         TMatrix.SetIdentity();
         T2Matrix.SetIdentity();
         T2Matrix._41 = -m_d.m_vCenter.x;
         T2Matrix._42 = -m_d.m_vCenter.y;
         T2Matrix._43 = -height;
         TMatrix._41 = m_d.m_vCenter.x;
         TMatrix._42 = m_d.m_vCenter.y;
         TMatrix._43 = height;

         tempMatrix.SetIdentity();
         tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotZ));
         tempMatrix.Multiply(RTmatrix, RTmatrix);
         tempMatrix.RotateYMatrix(ANGTORAD(m_d.m_rotY));
         tempMatrix.Multiply(RTmatrix, RTmatrix);
         tempMatrix.RotateXMatrix(ANGTORAD(m_d.m_rotX));
         tempMatrix.Multiply(RTmatrix, RTmatrix);
         for( int i=0;i<4;i++ )
         {      
            T2Matrix.MultiplyVector(lvertices[i], lvertices[i]);
            RTmatrix.MultiplyVector(lvertices[i], lvertices[i]);
            TMatrix.MultiplyVector(lvertices[i], lvertices[i]);
         }

         memcpy( buf, lvertices, sizeof(Vertex3D_NoLighting)*4 );
     
		 dynamicVertexBuffer->unlock();
      }

      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
      if ( m_d.m_fAddBlend )
      {
         const DWORD factor = (DWORD)(m_d.m_fAlpha<<24) + (DWORD)(m_d.m_fAlpha<<16) + (DWORD)(m_d.m_fAlpha<<8) + m_d.m_fAlpha;
         pd3dDevice->SetRenderState(RenderDevice::TEXTUREFACTOR, factor);
      }
      ppin3d->EnableAlphaBlend( 1, m_d.m_fAddBlend );

      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
      pd3dDevice->SetRenderState( RenderDevice::LIGHTING, FALSE );

      pd3dDevice->DrawPrimitiveVB( D3DPT_TRIANGLEFAN, dynamicVertexBuffer, 0, 4 );

      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
      pd3dDevice->SetRenderState( RenderDevice::LIGHTING, TRUE );
      if ( m_d.m_fAddBlend )
	  {
         pd3dDevice->SetRenderState(RenderDevice::TEXTUREFACTOR, 0xFFFFFFFF);
         pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	  }
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
      pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE); 	
}
