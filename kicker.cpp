// Kicker.cpp : Implementation of CVBATestApp and DLL registration.

#include "StdAfx.h"

////////////////////////////////////////////////////////////////////

Kicker::Kicker()
	{
	m_phitkickercircle = NULL;
	}

Kicker::~Kicker()
	{
	}

HRESULT Kicker::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
	{
	m_ptable = ptable;

	m_d.m_vCenter.x = x;
	m_d.m_vCenter.y = y;

	SetDefaults(fromMouseClick);

	return InitVBA(fTrue, 0, NULL);
	}

void Kicker::SetDefaults(bool fromMouseClick)
	{
	HRESULT hr;
	float fTmp;
	int iTmp;

	hr = GetRegStringAsFloat("DefaultProps\\Kicker","Radius", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_radius = fTmp;
	else
		m_d.m_radius = 25;

	hr = GetRegInt("DefaultProps\\Kicker","TimerEnabled", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
	else
		m_d.m_tdr.m_fTimerEnabled = false;
	
	hr = GetRegInt("DefaultProps\\Kicker","TimerInterval", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_tdr.m_TimerInterval = iTmp;
	else
		m_d.m_tdr.m_TimerInterval = 100;

	hr = GetRegInt("DefaultProps\\Kicker","Enabled", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_fEnabled = iTmp == 0 ? false : true;
	else
		m_d.m_fEnabled = fTrue;

	hr = GetRegStringAsFloat("DefaultProps\\Kicker","Scatter", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_scatter = fTmp;
	else
		m_d.m_scatter = 0;

	hr = GetRegStringAsFloat("DefaultProps\\Kicker","HitHeight", &fTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_hit_height = fTmp;
	else
		m_d.m_hit_height = 40;

	hr = GetRegString("DefaultProps\\Kicker", "Surface", &m_d.m_szSurface, MAXTOKEN);
	if ((hr != S_OK) || !fromMouseClick)
		m_d.m_szSurface[0] = 0;

	hr = GetRegInt("DefaultProps\\Kicker","KickerType", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_kickertype = (enum KickerType)iTmp;
	else
		m_d.m_kickertype = KickerHole;

	hr = GetRegInt("DefaultProps\\Kicker","Color", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_color = iTmp;
	else
		m_d.m_color = RGB(100,100,100);
	}

void Kicker::WriteRegDefaults()
	{
	char strTmp[40];

	SetRegValue("DefaultProps\\Kicker","Color",REG_DWORD,&m_d.m_color,4);
	SetRegValue("DefaultProps\\Kicker","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
	SetRegValue("DefaultProps\\Kicker","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
	SetRegValue("DefaultProps\\Kicker","Enabled",REG_DWORD,&m_d.m_fEnabled,4);
	sprintf_s(strTmp, 40, "%f", m_d.m_hit_height);
	SetRegValue("DefaultProps\\Kicker","HitHeight", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(strTmp, 40, "%f", m_d.m_radius);
	SetRegValue("DefaultProps\\Kicker","Radius", REG_SZ, &strTmp,strlen(strTmp));
	sprintf_s(strTmp, 40, "%f", m_d.m_scatter);
	SetRegValue("DefaultProps\\Kicker","Scatter", REG_SZ, &strTmp,strlen(strTmp));
	SetRegValue("DefaultProps\\Kicker","KickerType",REG_DWORD,&m_d.m_kickertype,4);
	SetRegValue("DefaultProps\\Kicker","Surface", REG_SZ, &m_d.m_szSurface,strlen(m_d.m_szSurface));
	}

void Kicker::PreRender(Sur * const psur)
	{
	}

void Kicker::Render(Sur * const psur)
	{
	psur->SetBorderColor(RGB(0,0,0),false,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);

	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.75f);
	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.5f);
	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.25f);
	}

void Kicker::GetTimers(Vector<HitTimer> * const pvht)
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

void Kicker::GetHitShapes(Vector<HitObject> * const pvho)
	{
	const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	KickerHitCircle * const phitcircle = new KickerHitCircle();

	phitcircle->m_pfe = NULL;

	phitcircle->center.x = m_d.m_vCenter.x;
	phitcircle->center.y = m_d.m_vCenter.y;
	phitcircle->radius = m_d.m_radius;
	phitcircle->zlow = height;
	
	phitcircle->zhigh = height + m_d.m_hit_height;	// height of kicker hit cylinder  

	phitcircle->m_zheight = height;		//height for Kicker locked ball + ball->radius

	phitcircle->m_fEnabled = m_d.m_fEnabled;

	phitcircle->m_ObjType = eKicker;
	phitcircle->m_pObj = (void*) this;

	phitcircle->m_pkicker = this;

	pvho->AddElement(phitcircle);

	m_phitkickercircle = phitcircle;
	}

void Kicker::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Kicker::EndPlay()
{
   m_phitkickercircle = NULL;

   IEditable::EndPlay();
}

void Kicker::PostRenderStatic(const RenderDevice* pd3dDevice)
{
}

void Kicker::RenderSetup(const RenderDevice* _pd3dDevice)
{
   if ((m_d.m_kickertype == KickerInvisible) || (m_d.m_kickertype == KickerHidden))
      return;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   ppin3d->ClearExtents(&recBounds, NULL, NULL);	

   const float inv_width  = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
   const float inv_height = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);

   for (int l=0;l<16;l++)
   {
      const float angle = (float)(M_PI*2.0/16.0)*(float)l;
      vertices[l].x = m_d.m_vCenter.x + sinf(angle)*m_d.m_radius;
      vertices[l].y = m_d.m_vCenter.y - cosf(angle)*m_d.m_radius;
      vertices[l].z = height + (0.1f - 30.0f);

      vertices[l+16].x = vertices[l].x;
      vertices[l+16].y = vertices[l].y;
      vertices[l+16].z = height + 0.1f;

      borderVerices[l].x = m_d.m_vCenter.x + sinf(angle)*(m_d.m_radius+6.0f);
      borderVerices[l].y = m_d.m_vCenter.y - cosf(angle)*(m_d.m_radius+6.0f);
      borderVerices[l].z = height + 0.05f;

      ppin3d->m_lightproject.CalcCoordinates(&vertices[l],inv_width,inv_height);
      ppin3d->m_lightproject.CalcCoordinates(&vertices[l+16],inv_width,inv_height);
      ppin3d->m_lightproject.CalcCoordinates(&borderVerices[l],inv_width,inv_height);
   }

   vertices[48].x = m_d.m_vCenter.x;
   vertices[48].y = m_d.m_vCenter.y;
   vertices[48].z = height + (0.1f - 30.0f);
   ppin3d->m_lightproject.CalcCoordinates(&vertices[48],inv_width,inv_height);

   ppin3d->EnableLightMap(fTrue, height);

   ppin3d->ExpandExtents(&recBounds, &vertices[16], NULL, NULL, 16, fFalse);
}


void Kicker::RenderStatic(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice = (RenderDevice*)_pd3dDevice;
   // Don't process "invisible" kickers.
   if ((m_d.m_kickertype == KickerInvisible) || (m_d.m_kickertype == KickerHidden))
      return;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);	
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
   g_pplayer->m_pin3d.SetColorKeyEnabled(FALSE);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);

   const float inv_width  = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
   const float inv_height = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);


   const float r = (m_d.m_color & 255) * (float)(1.0/255.0);
   const float g = (m_d.m_color & 65280) * (float)(1.0/65280.0);
   const float b = (m_d.m_color & 16711680) * (float)(1.0/16711680.0);
/*
   Material mtrl;
   mtrl.diffuse.a = 
   mtrl.ambient.a =
   mtrl.specular.r = mtrl.specular.g =	mtrl.specular.b = mtrl.specular.a =
   mtrl.emissive.r = mtrl.emissive.g =	mtrl.emissive.b = mtrl.emissive.a =
   mtrl.power = 0;
   mtrl.diffuse.r = mtrl.ambient.r = r;//0.7f;
   mtrl.diffuse.g = mtrl.ambient.g = g;//0.2f;
   mtrl.diffuse.b = mtrl.ambient.b = b;//0.2f;
   pd3dDevice->SetMaterial(&mtrl);
*/
   ppin3d->SetMaterial( 0, r, g, b );

   ppin3d->EnableLightMap(fTrue, height);

   if (m_d.m_kickertype == KickerCup)
   {
      // Draw outer ring
      WORD rgi[3*14];
      for (int l=0;l<14;l++)
      {
         rgi[l*3  ] = 0;
         rgi[l*3+1] = l+1;
         rgi[l*3+2] = l+2;

         SetNormal(borderVerices, rgi+l*3, 3, NULL, NULL, 0);
      }
      pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX,borderVerices,16,rgi, 3*14, 0);
   }

   Texture* pddsBufferBack = ppin3d->CreateOffscreen(recBounds.right - recBounds.left, recBounds.bottom - recBounds.top);
   Texture* pddsMask = ppin3d->CreateOffscreen(recBounds.right - recBounds.left, recBounds.bottom - recBounds.top);

   HRESULT hr = pddsBufferBack->BltFast(0, 0, ppin3d->m_pddsStatic, &recBounds, DDBLTFAST_WAIT);
   pd3dDevice->Clear( 1, (D3DRECT *)&recBounds, D3DCLEAR_TARGET, 0x00ffffff, 1.0f, 0L );

   ppin3d->SetMaterial( 0,0,0,0 );
/*   mtrl.diffuse.r = mtrl.ambient.r = 
   mtrl.diffuse.g = mtrl.ambient.g = 
   mtrl.diffuse.b = mtrl.ambient.b = 0.0f;
   pd3dDevice->SetMaterial(&mtrl);
*/
   ppin3d->EnableLightMap(fFalse, height);

   // Draw mask
   {
      WORD rgi[3*14];
      for (int l=0;l<14;l++)
      {
         rgi[l*3  ] = 0;
         rgi[l*3+1] = l+1;
         rgi[l*3+2] = l+2;

         SetNormal(vertices+16, rgi+l*3, 3, NULL, NULL, 0);
      }
      pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX,vertices+16, 16,rgi, 3*14, 0);
   }

   DDSURFACEDESC2 ddsd;
   ddsd.dwSize = sizeof(ddsd);
   DDSURFACEDESC2 ddsdMask;
   ddsdMask.dwSize = sizeof(ddsdMask);

   // Use mask to reset z-values underneath kicker
   hr = ppin3d->m_pddsStatic->Lock(&recBounds, &ddsdMask, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR 
      | DDLOCK_WAIT, NULL);
   if (hr == S_OK)
   {
      hr = ppin3d->m_pddsStaticZ->Lock(&recBounds, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR 
         | DDLOCK_WAIT, NULL);
      if (hr == S_OK)
      {
         const int colorbytes = ddsdMask.ddpfPixelFormat.dwRGBBitCount/8;
         const int zbytes = ddsd.ddpfPixelFormat.dwZBufferBitDepth/8;
         const int lenx = recBounds.right - recBounds.left;
         const int leny = recBounds.bottom - recBounds.top;
         const int pitch = ddsd.lPitch;
         BYTE *pch = (BYTE *)ddsd.lpSurface;

         const int pitchMask = ddsdMask.lPitch;
         const BYTE *pchMask = (BYTE *)ddsdMask.lpSurface;

         for (int y=0;y<leny;++y)
         {
            for (int x=0;x<lenx;++x)
            {
               if (*pchMask == 0)
                  for (int l=0;l<zbytes;++l)
                     pch[l] = 0xff;
               pch+=zbytes;
               pchMask+=colorbytes;
            }
            pch += pitch - lenx*zbytes;
            pchMask += pitchMask - lenx*colorbytes;
         }

         ppin3d->m_pddsStaticZ->Unlock(&recBounds);
      }
      ppin3d->m_pddsStatic->Unlock(&recBounds);
   }

   // Reset graphics around kicker
   hr = ppin3d->m_pddsStatic->Blt(&recBounds, pddsBufferBack, NULL, DDBLT_WAIT, NULL);

   // Draw the inside of the kicker based on its type.
   switch (m_d.m_kickertype)
   {
      case KickerHole: 
      {
         // Draw the kicker itself
         ppin3d->SetMaterial( 0,0,0,0 );
/*         
         mtrl.diffuse.r = mtrl.ambient.r =
         mtrl.diffuse.g = mtrl.ambient.g =
         mtrl.diffuse.b = mtrl.ambient.b = 0.0f;
         pd3dDevice->SetMaterial(&mtrl);
*/
         {
            WORD rgi[3*14];
            for (int l=0;l<14;++l)
            {
               rgi[l*3  ] = 0;
               rgi[l*3+1] = l+1;
               rgi[l*3+2] = l+2;

               SetNormal(vertices, rgi+l*3, 3, NULL, NULL, 0);
            }
            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX,vertices, 16,rgi, 3*14, 0);
         }
/*
         mtrl.diffuse.r = mtrl.ambient.r = r;//0.7f;
         mtrl.diffuse.g = mtrl.ambient.g = g;//0.7f;
         mtrl.diffuse.b = mtrl.ambient.b = b;//0.7f;
         pd3dDevice->SetMaterial(&mtrl);
*/
         ppin3d->SetMaterial( 0, r, g, b );
         ppin3d->EnableLightMap(fTrue, height);

         for (int l=0;l<16;++l)
         {
            const WORD rgiNormal[6] = {
               (l-1+16) % 16,
               (l-1+16) % 16 + 16,
               (l-1+16) % 16 + 2,
               l,
               l+16,
               (l+2) % 16};

               const WORD rgi[4] = {
                  l,
                  l+16,
                  (l+1) % 16 + 16,
                  (l+1) % 16};

                  SetNormal(vertices, rgiNormal, 3, NULL, rgi, 2);
                  SetNormal(vertices, &rgiNormal[3], 3, NULL, &rgi[2], 2);
                  pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,vertices, 32,(LPWORD)rgi, 4, 0);
         }
         break;
      }

      case KickerCup: 
      {
/*         mtrl.diffuse.r = mtrl.ambient.r = r;//0.7f;
         mtrl.diffuse.g = mtrl.ambient.g = g;//0.2f;
         mtrl.diffuse.b = mtrl.ambient.b = b;//0.2f;
         pd3dDevice->SetMaterial(&mtrl);
*/
         ppin3d->SetMaterial( 0, r, g, b );

         ppin3d->EnableLightMap(fTrue, height);

         WORD rgi[3*16];
         for (int l=0;l<16;++l)
         {
            rgi[l*3  ] = 32;
            rgi[l*3+1] = l;
            rgi[l*3+2] = (l + 1)%16;

            SetNormal(vertices+16, rgi+l*3, 3, NULL, NULL, 0);

            vertices[48].nx = 0;
            vertices[48].ny = 0;
            vertices[48].nz = -1.0f;
         }
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX,vertices+16, 49-16,rgi, 3*16, 0);
         break;
      }

      case KickerHidden:
      case KickerInvisible:
         break;
   }

   pddsBufferBack->Release();
   pddsMask->Release();

   ppin3d->EnableLightMap(fFalse, height);
}

void Kicker::RenderMovers(const RenderDevice* pd3dDevice)
{
}

void Kicker::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Kicker::MoveOffset(const float dx, const float dy)
	{
	m_d.m_vCenter.x += dx;
	m_d.m_vCenter.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Kicker::GetCenter(Vertex2D * const pv) const
	{
	*pv = m_d.m_vCenter;
	}

void Kicker::PutCenter(const Vertex2D * const pv)
	{
	m_d.m_vCenter = *pv;

	SetDirtyDraw();
	}


HRESULT Kicker::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
	bw.WriteFloat(FID(RADI), m_d.m_radius);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteInt(FID(COLR), m_d.m_color);
	bw.WriteString(FID(SURF), m_d.m_szSurface);
	bw.WriteBool(FID(EBLD), m_d.m_fEnabled);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
	bw.WriteInt(FID(TYPE), m_d.m_kickertype);
	bw.WriteFloat(FID(KSCT), m_d.m_scatter);
	bw.WriteFloat(FID(KHOT), m_d.m_hit_height);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	}

HRESULT Kicker::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults(false);
#ifndef OLDLOAD
	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
#else
	ULONG read = 0;
	HRESULT hr = S_OK;

	m_ptable = ptable;

	DWORD dwID;
	if(FAILED(hr = pstm->Read(&dwID, sizeof dwID, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_d, sizeof(KickerData), &read)))
		return hr;

	*pid = dwID;

	return hr;
#endif
	}

BOOL Kicker::LoadToken(int id, BiffReader *pbr)
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
	else if (id == FID(KSCT))
		{
		pbr->GetFloat(&m_d.m_scatter);
		}
	else if (id == FID(KHOT))
		{
		pbr->GetFloat(&m_d.m_hit_height);
		}
	else if (id == FID(COLR))
		{
		pbr->GetInt(&m_d.m_color);
//		if (!(m_d.m_color & MINBLACKMASK)) {m_d.m_color |= MINBLACK;}	// set minimum black
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(EBLD))
		{
		pbr->GetBool(&m_d.m_fEnabled);
		}
	else if (id == FID(TMIN))
		{
		pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(TYPE))
		{
		pbr->GetInt(&m_d.m_kickertype);
		}
	else if (id == FID(SURF))
		{
		pbr->GetString(m_d.m_szSurface);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else
		{
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT Kicker::InitPostLoad()
	{
	m_phitkickercircle = NULL;
	return S_OK;
	}


STDMETHODIMP Kicker::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_IKicker,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP Kicker::CreateSizedBall(/*[in]*/float radius, /*out, retval]*/ IBall **pBallEx)
{
	if (m_phitkickercircle)
		{
		
		const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

		Ball * const pball = g_pplayer->CreateBall(m_phitkickercircle->center.x,
				m_phitkickercircle->center.y, height, 0.1f, 0, 0, radius);

		*pBallEx = pball->m_pballex;
		pball->m_pballex->AddRef();

		pball->m_hitnormal[1].x = 1.0f;			//avoid capture leaving kicker
		m_phitkickercircle->Collide(pball, NULL); //
		}

	return S_OK;
}

STDMETHODIMP Kicker::CreateBall(IBall **pBallEx)
{
	if (m_phitkickercircle)
		{
		
		const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

		Ball * const pball = g_pplayer->CreateBall(m_phitkickercircle->center.x,
				m_phitkickercircle->center.y, height, 0.1f, 0, 0);

		*pBallEx = pball->m_pballex;
		pball->m_pballex->AddRef();

		pball->m_hitnormal[1].x = 1.0f;			//avoid capture leaving kicker
		m_phitkickercircle->Collide(pball, NULL); //
		}

	return S_OK;
}


STDMETHODIMP Kicker::DestroyBall(int *pVal)
{
	int cnt = 0;

	if (m_phitkickercircle && m_phitkickercircle->m_pball)
		{
		_ASSERTE(g_pplayer);
		++cnt;
		g_pplayer->DestroyBall(m_phitkickercircle->m_pball);

		m_phitkickercircle->m_pball = NULL;
		}

	if (pVal) *pVal = cnt;

	return S_OK;
}

STDMETHODIMP Kicker::KickXYZ(float angle, float speed, float inclination, float x, float y,float z)
{
	if (g_pplayer && m_phitkickercircle && m_phitkickercircle->m_pball)
		{
		float anglerad = ANGTORAD(angle);				// yaw angle, zero is along -Y axis		

		if (fabsf(inclination) > (float)(M_PI/2.0))		// radians or degrees?  if greater PI/2 assume degrees
			inclination *= (float)(M_PI/180.0);			// convert to radians
		
		float scatterAngle = (m_d.m_scatter <= 0.0f) ? c_hardScatter : ANGTORAD(m_d.m_scatter); // if <= 0 use global value
		scatterAngle *= g_pplayer->m_ptable->m_globalDifficulty;		// apply dificulty weighting

		if (scatterAngle > 1.0e-5f)										// ignore near zero angles
			{
			float scatter = rand_mt_m11();								// -1.0f..1.0f
			scatter *= (1.0f - scatter*scatter)*2.59808f * scatterAngle;// shape quadratic distribution and scale
			anglerad += scatter;
			}
		
		const float speedz = sinf(inclination) * speed;

		if (speedz > 0.f)
			speed = cosf(inclination) * speed;

		m_phitkickercircle->m_pball->x += x; // brian's suggestion
		m_phitkickercircle->m_pball->y += y; 
		m_phitkickercircle->m_pball->z += z; 

		m_phitkickercircle->m_pball->vx =  sinf(anglerad) * speed;
		m_phitkickercircle->m_pball->vy = -cosf(anglerad) * speed;
		m_phitkickercircle->m_pball->vz = speedz;
		m_phitkickercircle->m_pball->fFrozen = false;
		m_phitkickercircle->m_pball = NULL;
		}

	return S_OK;
}

STDMETHODIMP Kicker::KickZ(float angle, float speed, float inclination, float heightz)
{
	KickXYZ(angle, speed, inclination,0,0,heightz);
	return S_OK;	
}

STDMETHODIMP Kicker::Kick(float angle, float speed, float inclination)
{
	KickXYZ(angle, speed, inclination, 0,0,0);
	return S_OK;
}
		
STDMETHODIMP Kicker::get_X(float *pVal)
{
	*pVal = m_d.m_vCenter.x;
	return S_OK;
}

STDMETHODIMP Kicker::put_X(float newVal)
{
	STARTUNDO

	m_d.m_vCenter.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Kicker::get_Y(float *pVal)
{
	*pVal = m_d.m_vCenter.y;

	return S_OK;
}

STDMETHODIMP Kicker::put_Y(float newVal)
{
	STARTUNDO

	m_d.m_vCenter.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Kicker::get_Surface(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Kicker::put_Surface(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Kicker::get_Enabled(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_fEnabled);

	return S_OK;
}

STDMETHODIMP Kicker::put_Enabled(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fEnabled = VBTOF(newVal);

	if (m_phitkickercircle)
		{
		m_phitkickercircle->m_fEnabled = m_d.m_fEnabled;
		}

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Kicker::get_Scatter(float *pVal)
{
	*pVal = m_d.m_scatter;

	return S_OK;
}

STDMETHODIMP Kicker::put_Scatter(float newVal)
{
	STARTUNDO

	m_d.m_scatter = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Kicker::get_HitHeight(float *pVal)
{
	*pVal = m_d.m_hit_height;

	return S_OK;
}

STDMETHODIMP Kicker::put_HitHeight(float newVal)
{
	STARTUNDO

	m_d.m_hit_height = newVal;

	STOPUNDO

	return S_OK;
}


void Kicker::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPKICKER_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPKICKER_STATE, IDS_STATE);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}

STDMETHODIMP Kicker::get_DrawStyle(KickerType *pVal)
{
	*pVal = m_d.m_kickertype;

	return S_OK;
}

STDMETHODIMP Kicker::put_DrawStyle(KickerType newVal)
{
	STARTUNDO

	m_d.m_kickertype = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Kicker::get_Color(OLE_COLOR *pVal)
{
	*pVal = m_d.m_color;

	return S_OK;
}

STDMETHODIMP Kicker::put_Color(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_color = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Kicker::BallCntOver(int *pVal)
{
	int cnt = 0;

	if (g_pplayer)
		{
		const int vballsize = g_pplayer->m_vball.Size();

		for (int i = 0; i < vballsize; i++)
			{
			Ball * const pball = g_pplayer->m_vball.ElementAt(i);

			if (pball->m_vpVolObjs->IndexOf(this) >= 0)
				{
				++cnt;
				g_pplayer->m_pactiveball = pball;	// set active ball for scriptor
				}
			}		
		}

	*pVal = cnt;
	return S_OK;
}


KickerHitCircle::KickerHitCircle()
	{
	m_pball = NULL;
	}

void KickerHitCircle::Collide(Ball * const pball, Vertex3Ds * const phitnormal)
	{
	if (m_pball) return;								// a previous ball already in kicker

	const int i = pball->m_vpVolObjs->IndexOf(m_pObj);	// check if kicker in ball's volume set

	if (!phitnormal || ((phitnormal && phitnormal[1].x < 1) == (i < 0))) // New or (Hit && !Vol || UnHit && Vol)
		{		
		pball->x += pball->vx * STATICTIME; //move ball slightly forward
		pball->y += pball->vy * STATICTIME;
		pball->z += pball->vz * STATICTIME;

		if (i < 0)	//entering Kickers volume
			{ 
			pball->m_vpVolObjs->AddElement(m_pObj);		// add kicker to ball's volume set
				
			m_pball = pball;
			pball->fFrozen = true;			

			// Don't fire the hit event if the ball was just created
			// Fire the event before changing ball attributes, so scripters can get a useful ball state

			if (phitnormal) // pointer will be NULL if just created
				{m_pkicker->FireGroupEvent(DISPID_HitEvents_Hit);}
		
			if (pball->fFrozen)	// script may have unfrozen the ball
				{
				// Only mess with variables if ball was not kicked during event
				pball->vx = 0;
				pball->vy = 0;
				pball->vz = 0;
				pball->x = center.x;
				pball->y = center.y;
				pball->z = m_zheight + pball->radius;
				}
			else m_pball = NULL;		// make sure 
			}
		else // exiting kickers volume
			{				
			pball->m_vpVolObjs->RemoveElementAt(i);		// remove kicker to ball's volume set
			m_pkicker->FireGroupEvent(DISPID_HitEvents_Unhit);
			}	
		}	
	}
