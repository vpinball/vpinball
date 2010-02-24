// Kicker.cpp : Implementation of CVBATestApp and DLL registration.

#include "stdafx.h"
//#include "VBATest.h"
#include "main.h"

/////////////////////////////////////////////////////////////////////////////
//

Kicker::Kicker()
	{
	m_phitkickercircle = NULL;
	}

Kicker::~Kicker()
	{
	}

HRESULT Kicker::Init(PinTable *ptable, float x, float y)
	{
	m_ptable = ptable;

	m_d.m_vCenter.x = x;
	m_d.m_vCenter.y = y;

	SetDefaults();

	return InitVBA(fTrue, 0, NULL);
	}

void Kicker::SetDefaults()
	{
	m_d.m_radius = 25;

	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;
	m_d.m_fEnabled = fTrue;

	m_d.m_szSurface[0] = 0;

	m_d.m_kickertype = KickerHole;//KickerCup;

	m_d.m_color = RGB(100,100,100);
	}

void Kicker::PreRender(Sur *psur)
	{
	/*psur->SetBorderColor(-1,fFalse,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);

	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.75f);
	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.5f);
	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.25f);*/
	}

void Kicker::Render(Sur *psur)
	{
	psur->SetBorderColor(RGB(0,0,0),fFalse,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);

	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.75f);
	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.5f);
	psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.25f);
	}

void Kicker::GetTimers(Vector<HitTimer> *pvht)
	{
	IEditable::BeginPlay();

	HitTimer *pht;
	pht = new HitTimer();
	pht->m_interval = m_d.m_tdr.m_TimerInterval;
	pht->m_nextfire = pht->m_interval;
	pht->m_pfe = (IFireEvents *)this;

	m_phittimer = pht;

	if (m_d.m_tdr.m_fTimerEnabled)
		{
		pvht->AddElement(pht);
		}
	}

void Kicker::GetHitShapes(Vector<HitObject> *pvho)
	{
	KickerHitCircle *phitcircle;

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	phitcircle = new KickerHitCircle();

	phitcircle->m_pfe = NULL;

	phitcircle->center.x = m_d.m_vCenter.x;
	phitcircle->center.y = m_d.m_vCenter.y;
	phitcircle->radius = m_d.m_radius;
	phitcircle->zlow = height+0;
	phitcircle->zhigh = height+40;

	phitcircle->m_zheight = height;

	phitcircle->m_fEnabled = m_d.m_fEnabled;
	
	phitcircle->m_ObjType = eItemKicker;
	phitcircle->m_pObj = (void*) this;

	phitcircle->m_pkicker = this;

	pvho->AddElement(phitcircle);

	m_phitkickercircle = phitcircle;
	}

void Kicker::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}

void Kicker::EndPlay()
	{
	m_phitkickercircle = NULL;

	IEditable::EndPlay();
	}

void Kicker::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	int l;
	WORD rgi[8];
	Vertex3D rgv3D[49];
	Vertex3D rgvBorder[16];
	WORD rgiNormal[3];
	RECT rcBounds;
	LPDIRECTDRAWSURFACE7 pddsBufferBack;
	//LPDIRECTDRAWSURFACE7 pddsZBack;
	LPDIRECTDRAWSURFACE7 pddsMask;
	HRESULT hr;

	if (m_d.m_kickertype == KickerHidden)
		{
		return;
		}

	Pin3D *ppin3d = &g_pplayer->m_pin3d;

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

	ppin3d->ClearExtents(&rcBounds, NULL, NULL);

	//DWORD oldzfunc;
	//pd3dDevice->GetRenderState(D3DRENDERSTATE_ZFUNC, &oldzfunc);
	//pd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );

	float r = (m_d.m_color & 255) / 255.0f;
	float g = (m_d.m_color & 65280) / 65280.0f;
	float b = (m_d.m_color & 16711680) / 16711680.0f;

	for (l=0;l<16;l++)
		{
		float angle = PI*2;
		angle /= 16;
		angle *= l;
		rgv3D[l].x = (float)sin(angle)*m_d.m_radius + m_d.m_vCenter.x;
		rgv3D[l].y = (float)-cos(angle)*m_d.m_radius + m_d.m_vCenter.y;
		rgv3D[l].z = height + 0.1f - 30;

		rgv3D[l+16].x = rgv3D[l].x;
		rgv3D[l+16].y = rgv3D[l].y;
		rgv3D[l+16].z = height + 0.1f;

		rgvBorder[l].x = (float)sin(angle)*(m_d.m_radius+6) + m_d.m_vCenter.x;
		rgvBorder[l].y = (float)-cos(angle)*(m_d.m_radius+6) + m_d.m_vCenter.y;
		rgvBorder[l].z = height + 0.05f;

		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l+16]);
		ppin3d->m_lightproject.CalcCoordinates(&rgvBorder[l]);
		}

	rgv3D[48].x = m_d.m_vCenter.x;
	rgv3D[48].y = m_d.m_vCenter.y;
	rgv3D[48].z = height + 0.1f - 30;
	ppin3d->m_lightproject.CalcCoordinates(&rgv3D[48]);

	mtrl.diffuse.r = mtrl.ambient.r = r;//0.7f;
	mtrl.diffuse.g = mtrl.ambient.g = g;//0.2f;
	mtrl.diffuse.b = mtrl.ambient.b = b;//0.2f;
	pd3dDevice->SetMaterial(&mtrl);

	ppin3d->EnableLightMap(fTrue, height);

	if (m_d.m_kickertype != KickerHole)
		{
		// Draw outer ring
		for (l=1;l<15;l++)
				{
				rgi[0] = 0;
				rgi[1] = l;
				rgi[2] = l+1;

				SetNormal(rgvBorder, rgi, 3, NULL, NULL, 0);

				pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgvBorder, 16,
														  rgi, 3, NULL);
				}
		}

	ppin3d->ExpandExtents(&rcBounds, &rgv3D[16], NULL, NULL, 16, fFalse);
	pddsBufferBack = ppin3d->CreateOffscreen(rcBounds.right - rcBounds.left, rcBounds.bottom - rcBounds.top);
	//pddsZBack = ppin3d->CreateZBufferOffscreen(rcBounds.right - rcBounds.left, rcBounds.bottom - rcBounds.top);
	pddsMask = ppin3d->CreateOffscreen(rcBounds.right - rcBounds.left, rcBounds.bottom - rcBounds.top);

	hr = pddsBufferBack->Blt(NULL, ppin3d->m_pddsStatic, &rcBounds, 0, NULL);
	//hr = pddsZBack->BltFast(0, 0, ppin3d->m_pddsStaticZ, &rcBounds, DDBLTFAST_NOCOLORKEY);
	DDBLTFX ddbltfx;
	ddbltfx.dwSize = sizeof(DDBLTFX);
	ddbltfx.dwFillDepth = 0xffffffff;
	ddbltfx.ddckSrcColorkey.dwColorSpaceLowValue = 0;
	ddbltfx.ddckSrcColorkey.dwColorSpaceHighValue = 0;
	pd3dDevice->Clear( 1, (D3DRECT *)&rcBounds, D3DCLEAR_TARGET, 0x00ffffff, 1.0f, 0L );
	//ppin3d->m_pddsStaticZ->Blt(&rcBounds, NULL/*ppin3d->m_pddsStatic*/, &rcBounds, /*DDBLT_KEYSRCOVERRIDE |*/ /*DDBLT_KEYSRC |*/ DDBLT_DEPTHFILL, &ddbltfx);
	//pd3dDevice->Clear( 1, (D3DRECT *)&rcBounds, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );

	mtrl.diffuse.r = mtrl.ambient.r = 0.0f;
	mtrl.diffuse.g = mtrl.ambient.g = 0.0f;
	mtrl.diffuse.b = mtrl.ambient.b = 0.0f;
	pd3dDevice->SetMaterial(&mtrl);

	ppin3d->EnableLightMap(fFalse, height);

	// Draw mask
	for (l=1;l<15;l++)
			{
			rgi[0] = 16+0;
			rgi[1] = 16+l;
			rgi[2] = 16+l+1;

			SetNormal(rgv3D, rgi, 3, NULL, NULL, 0);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 32,
													  rgi, 3, NULL);
			}

	DDSURFACEDESC2 ddsd;
	ddsd.dwSize = sizeof(ddsd);
	DDSURFACEDESC2 ddsdMask;
	ddsdMask.dwSize = sizeof(ddsdMask);

	// Use mask to reset z-values underneath kicker
	hr = ppin3d->m_pddsStatic->Lock(&rcBounds, &ddsdMask, DDLOCK_READONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	if (hr == S_OK)
		{
		hr = ppin3d->m_pddsStaticZ->Lock(&rcBounds, &ddsd, DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);

		if (hr == S_OK)
			{
			int colorbytes;
			int zbytes;

			colorbytes = ddsdMask.ddpfPixelFormat.dwRGBBitCount/8;
			zbytes = ddsd.ddpfPixelFormat.dwZBufferBitDepth/8;
			int lenx, leny;
			lenx = rcBounds.right - rcBounds.left;
			leny = rcBounds.bottom - rcBounds.top;
			int x,y;
			int pitch;
			pitch = ddsd.lPitch;
			BYTE *pch = (BYTE *)ddsd.lpSurface;

			int pitchMask = ddsdMask.lPitch;
			BYTE *pchMask = (BYTE *)ddsdMask.lpSurface;

			for (y=0;y<leny;y++)
				{
				for (x=0;x<lenx;x++)
					{
					if (*pchMask == 0)
						{
						int l;
						for (l=0;l<zbytes;l++)
							{
							*pch++ = 0xff;
							}
						}
					else
						{
						pch+=zbytes;
						}
					pchMask+=colorbytes;
					}
				pch += pitch - lenx*zbytes;
				pchMask += pitchMask - lenx*colorbytes;
				}

			ppin3d->m_pddsStaticZ->Unlock(&rcBounds);
			}
		ppin3d->m_pddsStatic->Unlock(&rcBounds);
		}

	// Reset graphics around kicker
	hr = ppin3d->m_pddsStatic->Blt(&rcBounds, pddsBufferBack, NULL, 0, NULL);

	if (m_d.m_kickertype == KickerHole)
		{
		// Draw the kicker itself
		mtrl.diffuse.r = mtrl.ambient.r = 0.0f;
		mtrl.diffuse.g = mtrl.ambient.g = 0.0f;
		mtrl.diffuse.b = mtrl.ambient.b = 0.0f;
		pd3dDevice->SetMaterial(&mtrl);

		for (l=1;l<15;l++)
				{
				rgi[0] = 0;
				rgi[1] = l;
				rgi[2] = l+1;

				SetNormal(rgv3D, rgi, 3, NULL, NULL, 0);

				pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3D, 32,
														  rgi, 3, NULL);
				}

		mtrl.diffuse.r = mtrl.ambient.r = r;//0.7f;
		mtrl.diffuse.g = mtrl.ambient.g = g;//0.7f;
		mtrl.diffuse.b = mtrl.ambient.b = b;//0.7f;
		pd3dDevice->SetMaterial(&mtrl);

		ppin3d->EnableLightMap(fTrue, height);

		for (l=0;l<16;l++)
			{
			rgiNormal[0] = (l - 1 + 16) % 16;
			rgiNormal[1] = rgiNormal[0] + 16;
			rgiNormal[2] = rgiNormal[0] + 2;

			rgi[0] = l;
			rgi[1] = l+16;
			rgi[2] = (l+1) % 16 + 16;
			rgi[3] = ((l+1) % 16);

			SetNormal(rgv3D, rgiNormal, 3, NULL, rgi, 2);

			rgiNormal[0] = l;
			rgiNormal[1] = l+16;
			rgiNormal[2] = (l+2) % 16;

			SetNormal(rgv3D, rgiNormal, 3, NULL, &rgi[2], 2);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 32,
													  rgi, 4, NULL);
			}
		}
	else // KickerCup
		{
		mtrl.diffuse.r = mtrl.ambient.r = r;//0.7f;
		mtrl.diffuse.g = mtrl.ambient.g = g;//0.2f;
		mtrl.diffuse.b = mtrl.ambient.b = b;//0.2f;
		pd3dDevice->SetMaterial(&mtrl);

		ppin3d->EnableLightMap(fTrue, height);

		for (l=0;l<16;l++)
			{
			rgi[0] = 48;
			rgi[1] = l + 16;
			rgi[2] = ((l + 1)&15) + 16;

			SetNormal(rgv3D, rgi, 3, NULL, NULL, 0);

			rgv3D[48].nx = 0;
			rgv3D[48].ny = 0;
			rgv3D[48].nz = -1;

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 49,
													  rgi, 3, NULL);
			}
		}

	//hr = ppin3d->m_pddsStatic->Blt(&rcBounds, m_pdddBufferBack, NULL, 0, NULL);
	//hr = ppin3d->m_pddsStaticZ->BltFast(rcBounds.left, rcBounds.top, m_pdddZBack, NULL, DDBLTFAST_NOCOLORKEY);

	pddsBufferBack->Release();
	//pddsZBack->Release();
	pddsMask->Release();

	ppin3d->EnableLightMap(fFalse, height);

	//pd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, oldzfunc);

	//pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	}
	
void Kicker::RenderMoversFromCache(Pin3D *ppin3d)
	{
	}

void Kicker::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

void Kicker::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
	}

void Kicker::MoveOffset(float dx, float dy)
	{
	m_d.m_vCenter.x += dx;
	m_d.m_vCenter.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Kicker::GetCenter(Vertex *pv)
	{
	pv->x = m_d.m_vCenter.x;
	pv->y = m_d.m_vCenter.y;
	}

void Kicker::PutCenter(Vertex *pv)
	{
	m_d.m_vCenter.x = pv->x;
	m_d.m_vCenter.y = pv->y;

	m_ptable->SetDirtyDraw();
	}

HRESULT Kicker::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex));
	bw.WriteFloat(FID(RADI), m_d.m_radius);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteInt(FID(COLR), m_d.m_color);
	bw.WriteString(FID(SURF), m_d.m_szSurface);
	bw.WriteBool(FID(EBLD), m_d.m_fEnabled);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
	bw.WriteInt(FID(TYPE), m_d.m_kickertype);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	/*ULONG writ = 0;
	HRESULT hr = S_OK;

	DWORD dwID = ApcProjectItem.ID();
	if(FAILED(hr = pstm->Write(&dwID, sizeof dwID, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_d, sizeof(KickerData), &writ)))
		return hr;

	return hr;*/
	}

HRESULT Kicker::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults();
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

	//ApcProjectItem.Register(ptable->ApcProject, GetDispatch(), dwID);
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
		pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex));
		}
	else if (id == FID(RADI))
		{
		pbr->GetFloat(&m_d.m_radius);
		}
	else if (id == FID(COLR))
		{
		pbr->GetInt(&m_d.m_color);
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

/*void TriggerHitCircle::Collide(Ball *pball, Vertex *phitnormal)
	{
	m_ptrigger->FireVoidEvent(DISPID_HitEvents_Hit);
	}

STDMETHODIMP Trigger::get_Radius(float *pVal)
{
	*pVal = m_d.m_radius;

	return S_OK;
}

STDMETHODIMP Trigger::put_Radius(float newVal)
{
	m_d.m_radius = newVal;

	SetDirtyDraw();

	return S_OK;
}*/


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


STDMETHODIMP Kicker::CreateBall(IBall **pBallEx)
{
	if (m_phitkickercircle)
		{
		/*Ball *pball = new Ball();
		pball->radius = 25;
		g_pplayer->m_vball.AddElement(pball);*/

		float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

		Ball *pball = g_pplayer->CreateBall(m_phitkickercircle->center.x,
				m_phitkickercircle->center.y, height, 0.1f, 0, 0);

		*pBallEx = pball->m_pballex;
		pball->m_pballex->AddRef();

		m_phitkickercircle->Collide(pball, NULL);
		}

	return S_OK;
}

STDMETHODIMP Kicker::DestroyBall()
{
	if (m_phitkickercircle && m_phitkickercircle->m_pball)
		{
		_ASSERTE(g_pplayer);

		g_pplayer->DestroyBall(m_phitkickercircle->m_pball);

		m_phitkickercircle->m_pball = NULL;
		}

	return S_OK;
}


STDMETHODIMP Kicker::Kick(float angle, float speed, float inclination)
{
	if (m_phitkickercircle && m_phitkickercircle->m_pball)
		{
		_ASSERTE(g_pplayer);
		float anglerad = angle/360*PI*2;
		float anglezrad = inclination;///360*PI*2;

		float speedz = ((float)sin(anglezrad)) * speed;
		speed = ((float)cos(anglezrad))*speed;

		m_phitkickercircle->m_pball->vx = (float)(sin(anglerad) * speed);
		m_phitkickercircle->m_pball->vy = (float)(-cos(anglerad) * speed);

		if (speedz > 0)
			{
			m_phitkickercircle->m_pball->vz = speedz;
			//m_phitkickercircle->m_pball->m_plevel = NULL;
			}

		m_phitkickercircle->m_pball->fFrozen = fFalse;
		m_phitkickercircle->m_pball = NULL;
		}

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
	OLECHAR wz[512];

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
	*pVal = FTOVB(m_d.m_fEnabled);

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

/*int Kicker::GetDialogID()
	{
	return IDD_PROPKICKER;
	}*/

void Kicker::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPKICKER_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPTRIGGER_STATE, IDS_STATE);
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

KickerHitCircle::KickerHitCircle()
	{
	m_pball = NULL;
	}

void KickerHitCircle::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (m_pball)
		{
		return; // Can't capture 2 balls at once
		}

	if (!phitnormal || phitnormal[1].x < 1) // Only hit, no Unhit event
		{
		m_pball = pball;

		pball->fFrozen = fTrue;

		// Don't fire the hit event if the ball was just created
		// Fire the event before changing ball attributes, so scripters can get a useful ball state
		if (phitnormal)
			{
			m_pkicker->FireGroupEvent(DISPID_HitEvents_Hit);
			}

		if (pball->fFrozen)
			{
			// Only mess with variables if ball was not kicked during event
			pball->vx = 0;
			pball->vy = 0;
			pball->vz = 0;
			pball->x = center.x;
			pball->y = center.y;
			pball->z = m_zheight + pball->radius;
			}
		}
	}

/*HRESULT Kicker::GetTypeName(BSTR *pVal)
	{
	*pVal = SysAllocString(L"Kicker");

	return S_OK;
	}*/
