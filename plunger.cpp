// Plunger.cpp : Implementation of CVBATestApp and DLL registration.

#include "stdafx.h"
#include "main.h"

/////////////////////////////////////////////////////////////////////////////
//

Plunger::Plunger()
	{
	m_phitplunger = NULL;
	}

Plunger::~Plunger()
	{
	}

HRESULT Plunger::Init(PinTable *ptable, float x, float y)
	{
	m_ptable = ptable;

	m_d.m_v.x = x;
	m_d.m_v.y = y;

	SetDefaults();

	return InitVBA(fTrue, 0, NULL);
	}

void Plunger::SetDefaults()
	{
	m_d.m_width = 25;
	m_d.m_height = 20;

	m_d.m_heightPulled = 0;

	m_d.m_speedPull = 5;
	m_d.m_speedFire = 80;
	m_d.m_powerBias = 0;					//>>> added by chris

	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;

	m_d.m_szSurface[0] = 0;
	
	/* BEGIN NEW PHYSICS */
	m_d.m_stroke = m_d.m_height*4;
	m_d.m_mechPlunger = fFalse;		//rlc plungers require selection for mechanical input
	m_d.m_autoPlunger = fFalse;		
	m_d.m_mechStrength = 85;		//rlc
	m_d.m_parkPosition = 0.5f/3.0f;	// typical mechanical plunger has 3 inch stroke and 0.5 inch rest position

	m_d.m_scatterVelocity = 0;
	m_d.m_breakOverVelocity = 18.0f;


	/* END NEW PHYSICS */
	}

void Plunger::PreRender(Sur *psur)
	{
	}

void Plunger::Render(Sur *psur)
	{
	psur->SetBorderColor(RGB(0,0,0),fFalse,0);
	psur->SetFillColor(-1);
	psur->SetObject(this);

	psur->Rectangle(m_d.m_v.x - m_d.m_width, m_d.m_v.y - m_d.m_stroke,
			m_d.m_v.x + m_d.m_width, m_d.m_v.y + m_d.m_height);
	}

void Plunger::GetHitShapes(Vector<HitObject> *pvho)
	{
	HitPlunger *php;

	float zheight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_v.x, m_d.m_v.y);

	php = new HitPlunger(m_d.m_v.x - m_d.m_width, m_d.m_v.y + m_d.m_height, m_d.m_v.x + m_d.m_width,
			m_d.m_autoPlunger ? m_d.m_v.y - m_d.m_heightPulled : m_d.m_v.y - m_d.m_stroke, zheight, 
			m_d.m_parkPosition, m_d.m_scatterVelocity, m_d.m_breakOverVelocity, m_d.m_mechStrength,m_d.m_mechPlunger);

	php->m_pfe = NULL;

	php->m_plungeranim.m_frameStart = m_d.m_v.y; //- m_d.m_heightPulled;
	php->m_plungeranim.m_frameEnd = m_d.m_v.y - m_d.m_stroke;//m_d.m_height*4;

	pvho->AddElement(php);
	php->m_pplunger = this;
	m_phitplunger = php;
	}

void Plunger::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}

void Plunger::GetTimers(Vector<HitTimer> *pvht)
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

void Plunger::EndPlay()
	{
	int i;

	if (m_phitplunger) // Failed Player case
		{
		for (i=0;i<m_phitplunger->m_plungeranim.m_vddsFrame.Size();i++)
			{
			//m_phitplunger->m_vddsFrame.ElementAt(i)->ppds->Release();
			delete m_phitplunger->m_plungeranim.m_vddsFrame.ElementAt(i);
			}

		m_phitplunger = NULL;
		}

	IEditable::EndPlay();
	}

void Plunger::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_v.x, m_d.m_v.y);
	}

void Plunger::MoveOffset(float dx, float dy)
	{
	m_d.m_v.x += dx;
	m_d.m_v.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Plunger::GetCenter(Vertex *pv)
	{
	pv->x = m_d.m_v.x;
	pv->y = m_d.m_v.y;
	}

void Plunger::PutCenter(Vertex *pv)
	{
	m_d.m_v.x = pv->x;
	m_d.m_v.y = pv->y;

	m_ptable->SetDirtyDraw();
	}

void Plunger::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

#define PLUNGEPOINTS 5

float rgcrossplunger[][2] = {
	1, 0,
	1, 10,
	0.35f, 20,
	0.35f, 24,
	0.35f, 100,
	};

float rgcrossplungerNormal[][2] = {
	1, 0,
	0.8f, 0.6f,
	0, 1,
	1, 0,
	1, 0,
	};
	
#define PLUNGER_FRAME_COUNT 10

void Plunger::RenderMoversFromCache(Pin3D *ppin3d)
	{
	ppin3d->ReadAnimObjectFromCacheFile(&m_phitplunger->m_plungeranim, &m_phitplunger->m_plungeranim.m_vddsFrame);
	}

void Plunger::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	_ASSERTE(m_phitplunger);
	Pin3D *ppin3d = &g_pplayer->m_pin3d;
	LPDIRECTDRAWSURFACE7 pdds;
	//DDBLTFX ddbfx;
	ObjFrame *pof;

	float zheight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_v.x, m_d.m_v.y);

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = 0.3f;
	mtrl.diffuse.g = mtrl.ambient.g = 0.3f;
	mtrl.diffuse.b = mtrl.ambient.b = 0.3f;
	pd3dDevice->SetMaterial(&mtrl);

	int cframes = (int)((float)PLUNGER_FRAME_COUNT * (m_d.m_stroke/80.0f)) + 1; // 25 frames per 80 units travel

	int i;

	//ddbfx.dwSize = sizeof(DDBLTFX);
	//ddbfx.dwFillColor = RGB(0,0,0);

	//float anglerad = ANGTORAD(m_d.m_StartAngle);
	//float anglerad2 = ANGTORAD(m_d.m_EndAngle);

	float beginy = m_d.m_v.y;// - m_d.m_heightPulled;
	float endy = m_d.m_v.y - m_d.m_stroke;//m_d.m_height*4;

	WORD rgi[8];

	Vertex3D rgv3D[16*PLUNGEPOINTS];

	ppin3d->ClearExtents(&m_phitplunger->m_plungeranim.m_rcBounds, &m_phitplunger->m_plungeranim.m_znear, &m_phitplunger->m_plungeranim.m_zfar);

	for (i=0;i<cframes;i++)
		{
		float height = beginy + (endy - beginy)/(cframes-1)*i;

		pof = new ObjFrame();

		//ppin3d->m_pddsBackBuffer->Blt(NULL, ppin3d->m_pddsStatic, NULL, 0, NULL);
		//ppin3d->m_pddsZBuffer->Blt(NULL, ppin3d->m_pddsStaticZ, NULL, 0, NULL);

		int l,m;
		for (l=0;l<16;l++)
			{
			float angle = PI*2;
			angle /= 16;
			angle *= l;

			float sn,cs;

			sn = (float)sin(angle);
			cs = (float)cos(angle);

			int offset = l*PLUNGEPOINTS;

			for (m=0;m<PLUNGEPOINTS;m++)
				{
				rgv3D[m + offset].x = rgcrossplunger[m][0] * sn * m_d.m_width + m_d.m_v.x;
				rgv3D[m + offset].y = height + rgcrossplunger[m][1];
				rgv3D[m + offset].z = rgcrossplunger[m][0] * cs * m_d.m_width + m_d.m_width + zheight;
				rgv3D[m + offset].nx = rgcrossplungerNormal[m][0] * sn;
				rgv3D[m + offset].ny = rgcrossplungerNormal[m][1];
				rgv3D[m + offset].nz = -rgcrossplungerNormal[m][0] * cs;
				}

			rgv3D[PLUNGEPOINTS-1 + offset].y = m_d.m_v.y + m_d.m_height; // cuts off at bottom (bottom of shaft disappears)
			}

		/*int l;
		for (l=0;l<8;l++)
			{
			rgv3D[l].x = (l&1) ? (m_d.m_v.x - m_d.m_width) : (m_d.m_v.x + m_d.m_width);
			rgv3D[l].y = (l&2) ? height + 20 : height;
			rgv3D[l].z = (l&4) ? 50.0f : 0.02f;

			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
			}*/

		ppin3d->ClearExtents(&pof->rc, NULL, NULL);
		ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitplunger->m_plungeranim.m_znear, &m_phitplunger->m_plungeranim.m_zfar, (16*PLUNGEPOINTS), fFalse);

		for (l=0;l<16;l++)
			{
			int offset = l*PLUNGEPOINTS;

			for (m=0;m<(PLUNGEPOINTS-1);m++)
				{
				rgi[0] = m + offset;
				rgi[3] = m + 1 + offset;
				rgi[2] = (m + 1 + offset + PLUNGEPOINTS) % (16*PLUNGEPOINTS);
				rgi[1] = (m + offset + PLUNGEPOINTS) % (16*PLUNGEPOINTS);

				//SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

				pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  rgv3D, (16*PLUNGEPOINTS),
															  rgi, 4, NULL);
				}
			}

		/*rgi[0] = 0;
		rgi[1] = 1;
		rgi[2] = 3;
		rgi[3] = 2;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 4, NULL);

		rgi[0] = 4;
		rgi[3] = 5;
		rgi[2] = 7;
		rgi[1] = 6;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 4, NULL);

		rgi[0] = 1;
		rgi[3] = 3;
		rgi[2] = 7;
		rgi[1] = 5;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 4, NULL);

		rgi[0] = 2;
		rgi[1] = 3;
		rgi[2] = 7;
		rgi[3] = 6;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 4, NULL);*/

		pdds = ppin3d->CreateOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
		pof->pddsZBuffer = ppin3d->CreateZBufferOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);

		pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &pof->rc, 0, NULL);
		pof->pddsZBuffer->BltFast(0, 0, ppin3d->m_pddsZBuffer, &pof->rc, DDBLTFAST_NOCOLORKEY);

		//pdds->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbfx);
		m_phitplunger->m_plungeranim.m_vddsFrame.AddElement(pof);
		pof->pdds = pdds;
		
		//ppin3d->WriteObjFrameToCacheFile(pof);

		ppin3d->ExpandRectByRect(&m_phitplunger->m_plungeranim.m_rcBounds, &pof->rc);

		// reset the portion of the z-buffer that we changed
		ppin3d->m_pddsZBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY);
		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;
		ppin3d->m_pddsBackBuffer->Blt(&pof->rc, NULL,
				&pof->rc, DDBLT_COLORFILL, &ddbltfx);
		}
		
		ppin3d->WriteAnimObjectToCacheFile(&m_phitplunger->m_plungeranim, &m_phitplunger->m_plungeranim.m_vddsFrame);
	}

STDMETHODIMP Plunger::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_IPlunger,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

HRESULT Plunger::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_v, sizeof(Vertex));
	bw.WriteFloat(FID(WDTH), m_d.m_width);
	bw.WriteFloat(FID(HIGH), m_d.m_height);
	bw.WriteFloat(FID(HGHP), m_d.m_heightPulled);
	bw.WriteFloat(FID(SPDP), m_d.m_speedPull);
	bw.WriteFloat(FID(SPDF), m_d.m_speedFire);
	bw.WriteFloat(FID(PWRB), m_d.m_powerBias);		//>>> added by chris

	bw.WriteFloat(FID(HPSL), m_d.m_stroke);
	bw.WriteFloat(FID(MESTH), m_d.m_mechStrength);		//rlc
	bw.WriteBool(FID(MECH), m_d.m_mechPlunger);		//
	bw.WriteBool(FID(APLG), m_d.m_autoPlunger);		//

	bw.WriteFloat(FID(MPRK), m_d.m_parkPosition);		//
	bw.WriteFloat(FID(PSCV), m_d.m_scatterVelocity);	//
	bw.WriteFloat(FID(PBOV), m_d.m_breakOverVelocity);	//


	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteString(FID(SURF), m_d.m_szSurface);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	/*ULONG writ = 0;
	HRESULT hr = S_OK;

	DWORD dwID = ApcProjectItem.ID();
	if(FAILED(hr = pstm->Write(&dwID, sizeof dwID, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_d, sizeof(PlungerData), &writ)))
		return hr;

	return hr;*/
	}

HRESULT Plunger::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
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

	if(FAILED(hr = pstm->Read(&m_d, sizeof(PlungerData), &read)))
		return hr;

	*pid = dwID;

	return hr;
#endif
	}

BOOL Plunger::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VCEN))
		{
		pbr->GetStruct(&m_d.m_v, sizeof(Vertex));
		}
	else if (id == FID(WDTH))
		{
		pbr->GetFloat(&m_d.m_width);
		m_d.m_width = 25;
		}
	else if (id == FID(HIGH))
		{
		pbr->GetFloat(&m_d.m_height);
		}
	else if (id == FID(HGHP))
		{
		pbr->GetFloat(&m_d.m_heightPulled);
		}
	else if (id == FID(SPDP))
		{
		pbr->GetFloat(&m_d.m_speedPull);
		}
	else if (id == FID(SPDF))
		{
		pbr->GetFloat(&m_d.m_speedFire);
		}
	else if (id == FID(PWRB))
		{
		pbr->GetFloat(&m_d.m_powerBias);
		}
	else if (id == FID(TMON))
		{
		pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
		}
	else if (id == FID(TMIN))
		{
		pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(SURF))
		{
		pbr->GetString(m_d.m_szSurface);
		}
	else if (id == FID(HPSL))
		{
			pbr->GetFloat(&m_d.m_stroke);
		}
	else if (id == FID(MESTH))
		{
		pbr->GetFloat(&m_d.m_mechStrength);
		}
	else if (id == FID(MPRK))
		{
		pbr->GetFloat(&m_d.m_parkPosition);
		}
	else if (id == FID(PSCV))
		{
		pbr->GetFloat(&m_d.m_scatterVelocity);
		}
	else if (id == FID(PBOV))
		{
		pbr->GetFloat(&m_d.m_breakOverVelocity);
		}
	else if (id == FID(MECH))
		{
		pbr->GetBool(&m_d.m_mechPlunger);
		}
	else if (id == FID(APLG))
		{
		pbr->GetBool(&m_d.m_autoPlunger);
		}

	return fTrue;
	}

HRESULT Plunger::InitPostLoad()
	{
	return S_OK;
	}

STDMETHODIMP Plunger::PullBack()
{
	if (m_phitplunger)
		{
		m_phitplunger->m_plungeranim.m_posdesired = m_d.m_v.y - m_d.m_heightPulled;
		m_phitplunger->m_plungeranim.m_speed = /*m_d.m_speedPull*/0;
		m_phitplunger->m_plungeranim.m_force = m_d.m_speedPull;
		//m_phitplunger->m_acc = 0;
		m_phitplunger->m_plungeranim.m_fAcc = fTrue;

/*#ifdef LOG
		{
		int i = g_pplayer->m_vmover.IndexOf(m_phitplunger);
		fprintf(g_pplayer->m_flog, "Plunger Pull %d\n", i);
		}
#endif*/
		}

	return S_OK;
}

STDMETHODIMP Plunger::Fire()
{
	if (m_phitplunger)
	{
		// Check if this is an auto-plunger.
		// They always use max strength (as in a button).
		if ( m_d.m_autoPlunger == fTrue ) 
		{
			// Use max strength.
			m_phitplunger->m_plungeranim.m_posdesired = m_d.m_v.y - m_d.m_stroke; 
			m_phitplunger->m_plungeranim.m_speed = 0;
			m_phitplunger->m_plungeranim.m_force = -m_d.m_speedFire;;//-m_d.m_mechStrength;

			m_phitplunger->m_plungeranim.m_fAcc = fTrue;			
			m_phitplunger->m_plungeranim.m_mechTimeOut = 0; 
		}
		else
		{
			m_phitplunger->m_plungeranim.m_posdesired = m_d.m_v.y - m_d.m_stroke;
			m_phitplunger->m_plungeranim.m_speed = 0;
			m_phitplunger->m_plungeranim.m_force = -m_d.m_speedFire;
			m_phitplunger->m_plungeranim.m_fAcc = fTrue;	
		}
	}

#ifdef LOG
		int i = g_pplayer->m_vmover.IndexOf(m_phitplunger);
		fprintf(g_pplayer->m_flog, "Plunger Release %d\n", i);
#endif

	return S_OK;
}

STDMETHODIMP Plunger::get_PullSpeed(float *pVal)
{
	*pVal = m_d.m_speedPull;

	return S_OK;
}

STDMETHODIMP Plunger::put_PullSpeed(float newVal)
{
	STARTUNDO

	m_d.m_speedPull = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_FireSpeed(float *pVal)
{
	*pVal = m_d.m_speedFire;

	return S_OK;
}

STDMETHODIMP Plunger::put_FireSpeed(float newVal)
{
	STARTUNDO

	m_d.m_speedFire = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::CreateBall(IBall **pBallEx)
{
	if (m_phitplunger)
		{
		float radius = 25;
		float x,y;

		x = (m_phitplunger->m_plungeranim.m_x + m_phitplunger->m_plungeranim.m_x2) / 2;
		y = m_phitplunger->m_plungeranim.m_pos - radius - 0.01f;

		float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, x, y);

		Ball *pball = g_pplayer->CreateBall(x, y, height, 0.1f, 0, 0);
		
		/*Ball *pball = new Ball();
		pball->radius = 25;
		pball->x = (m_phitplunger->m_x + m_phitplunger->m_x2) / 2;
		pball->y = m_phitplunger->m_pos - pball->radius - 0.01f;
		pball->vx = 0.1f;
		pball->vy = 0;
		g_pplayer->m_vball.AddElement(pball);*/

		*pBallEx = pball->m_pballex;
		pball->m_pballex->AddRef();
		}

	return S_OK;
}

STDMETHODIMP Plunger::get_X(float *pVal)
{
	*pVal = m_d.m_v.x;

	return S_OK;
}

STDMETHODIMP Plunger::put_X(float newVal)
{
	STARTUNDO

	m_d.m_v.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_Y(float *pVal)
{
	*pVal = m_d.m_v.y;

	return S_OK;
}

STDMETHODIMP Plunger::put_Y(float newVal)
{
	STARTUNDO

	m_d.m_v.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_Surface(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Plunger::put_Surface(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

/*HRESULT Plunger::GetTypeName(BSTR *pVal)
	{
	*pVal = SysAllocString(L"Plunger");

	return S_OK;
	}*/

STDMETHODIMP Plunger::get_PowerBias(float *pVal)
{
	*pVal = m_d.m_powerBias;

	return S_OK;
}

STDMETHODIMP Plunger::put_PowerBias(float newVal)
{
	STARTUNDO
	m_d.m_powerBias = newVal;
	STOPUNDO

	return S_OK;
}

STDMETHODIMP Plunger::get_MechPlunger(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_mechPlunger);

	return S_OK;
}

STDMETHODIMP Plunger::put_MechPlunger(VARIANT_BOOL newVal)
{
	STARTUNDO
		m_d.m_mechPlunger = VBTOF(newVal);
	STOPUNDO

		return S_OK;
}


STDMETHODIMP Plunger::get_AutoPlunger(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_autoPlunger);

	return S_OK;
}

STDMETHODIMP Plunger::put_AutoPlunger(VARIANT_BOOL newVal)
{
	STARTUNDO
		m_d.m_autoPlunger = VBTOF(newVal);
	STOPUNDO

		return S_OK;
}


STDMETHODIMP Plunger::get_ParkPosition(float *pVal)
{
	*pVal = m_d.m_parkPosition;
	return S_OK;
}

STDMETHODIMP Plunger::put_ParkPosition(float newVal)
{
	STARTUNDO
	m_d.m_parkPosition = newVal;
	STOPUNDO

		return S_OK;
}

STDMETHODIMP Plunger::get_Stroke(float *pVal)
{
	*pVal = m_d.m_stroke;
	return S_OK;
}

STDMETHODIMP Plunger::put_Stroke(float newVal)
{
	STARTUNDO

	if(newVal < 16.5f)
		newVal = 16.5f;
	
	m_d.m_stroke = newVal;

	STOPUNDO

		return S_OK;
}


STDMETHODIMP Plunger::get_ScatterVelocity(float *pVal)
{
	*pVal = m_d.m_scatterVelocity;
	return S_OK;
}

STDMETHODIMP Plunger::put_ScatterVelocity(float newVal)
{
	STARTUNDO
		m_d.m_scatterVelocity = newVal;
	STOPUNDO

		return S_OK;
}

STDMETHODIMP Plunger::get_BreakOverVelocity(float *pVal)
{
	*pVal = m_d.m_breakOverVelocity;
	return S_OK;
}

STDMETHODIMP Plunger::put_BreakOverVelocity(float newVal)
{
	STARTUNDO
	m_d.m_breakOverVelocity = newVal;
	STOPUNDO

		return S_OK;
}

STDMETHODIMP Plunger::get_MechStrength(float *pVal)
{
	*pVal = m_d.m_mechStrength;

	return S_OK;
}

STDMETHODIMP Plunger::put_MechStrength(float newVal)
{
	STARTUNDO
		m_d.m_mechStrength = newVal;
	STOPUNDO

		return S_OK;
}

void Plunger::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPPLUNGER_PHYSICS, IDS_STATE);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}