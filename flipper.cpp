// Flipper.cpp : Implementation of CVBATestApp and DLL registration.

#include "stdafx.h"
//#include "VBATest.h"
#include "main.h"

/////////////////////////////////////////////////////////////////////////////
//

Flipper::Flipper()
	{
	m_phitflipper = NULL;
	}

Flipper::~Flipper()
	{
	}

HRESULT Flipper::Init(PinTable *ptable, float x, float y)
	{
	m_ptable = ptable;

	m_d.m_Center.x = x;
	m_d.m_Center.y = y;

	SetDefaults();

	m_phitflipper = NULL;

	return InitVBA(fTrue, 0, NULL);
	}

void Flipper::SetDefaults()
	{
	//m_d.m_BaseRadius = 15;
	//m_d.m_EndRadius = 6;
	m_d.m_StartAngle = 120;
	m_d.m_EndAngle = 60;

	m_d.m_BaseRadius = 26.73f; // 15
	m_d.m_EndRadius = 10.69f; // 6
	m_d.m_FlipperRadius = 142.57f; // 80

	m_d.m_force = 0.05f;
	m_d.m_elasticity = 0.3f;

	m_d.m_tdr.m_fTimerEnabled = fFalse;
	m_d.m_tdr.m_TimerInterval = 100;

	m_d.m_color = RGB(255,255,255);
	m_d.m_rubbercolor = RGB(128,128,128);

	m_d.m_rubberthickness = 0;

	m_d.m_szSurface[0] = 0;

	m_d.m_strength = 6;
	m_d.m_returnstrength = 6.0f;

	m_d.m_fVisible = fTrue;
	
	/* Being New Phys Stuff */
	m_d.m_returnforce = 0.05f;
	m_d.m_FlipperRadiusMin = 0;
	//m_d.m_FlipperRadiusMax = m_d.m_FlipperRadius;
	m_d.m_recoil = 0;		// disabled

	m_d.m_angleEOS = 0;		//disabled

	m_d.m_return = 1;		// match existing physics, return equals stroke 

	//m_d.m_returnforce = 0.05f;

	m_d.m_friction = 0;	//zero uses global value
	m_d.m_scatter = 0;	//zero uses global value

	//m_d.m_returnstrength = 6.0f;

	m_d.m_powerlaw = 2.0f;

	m_d.m_baseobliquecorrection = 0.0f; //flipper face correction 
	m_d.m_obliquecorrection = 0.0f; //flipper face correction 
	m_d.m_tipobliquecorrection = 0.0f; //flipper face correction 
	m_d.m_scatterangle = 0.0f; //flipper scatter angle

	m_d.m_height = 50;

	m_d.m_rubberthickness = 0;
	//m_d.m_rubberheight = 8;
	//m_d.m_rubberwidth = (int) (m_d.m_height - 16);

	m_d.m_mass = 1;

	/* End New Phys Stuff */
	}

void Flipper::GetTimers(Vector<HitTimer> *pvht)
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

void Flipper::GetHitShapes(Vector<HitObject> *pvho)
	{
	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

	HitFlipper *phf;
	phf = new HitFlipper(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, m_d.m_EndRadius,
		m_d.m_FlipperRadius, ANGTORAD(m_d.m_StartAngle), height, height+50, m_d.m_strength, m_d.m_mass);

	phf->m_elasticity = m_d.m_elasticity;

	phf->m_pfe = NULL;

	phf->m_flipperanim.m_frameStart = ANGTORAD(m_d.m_StartAngle);
	phf->m_flipperanim.m_frameEnd = ANGTORAD(m_d.m_EndAngle);
	phf->m_flipperanim.m_angleMin = min(phf->m_flipperanim.m_frameStart, phf->m_flipperanim.m_frameEnd);
	phf->m_flipperanim.m_angleMax = max(phf->m_flipperanim.m_frameStart, phf->m_flipperanim.m_frameEnd);

	phf->m_flipperanim.m_fEnabled = m_d.m_fVisible;

	pvho->AddElement(phf);
	phf->m_pflipper = this;
	m_phitflipper = phf;
	}

void Flipper::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

	HitObject *pho = CreateCircularHitPoly(m_d.m_Center.x, m_d.m_Center.y, height + 50, m_d.m_FlipperRadius + m_d.m_EndRadius, 32);
	pvho->AddElement(pho);
	}

void Flipper::EndPlay()
	{
	int i;

	if (m_phitflipper) // Failed player case
		{
		for (i=0;i<m_phitflipper->m_flipperanim.m_vddsFrame.Size();i++)
			{
			//m_phitflipper->m_vddsFrame.ElementAt(i)->ppds->Release();
			delete m_phitflipper->m_flipperanim.m_vddsFrame.ElementAt(i);
			}

		m_phitflipper = NULL;
		}

	IEditable::EndPlay();
	}

void Flipper::SetVertices(float angle, Vertex *pvEndCenter, Vertex *rgvTangents, float baseradius, float endradius)
	{
	double a,b,c;
	double r,r2;
	double x,y,x2,y2;

	pvEndCenter->x = (float)(m_d.m_Center.x + m_d.m_FlipperRadius*sin(angle));
	pvEndCenter->y = (float)(m_d.m_Center.y - m_d.m_FlipperRadius*cos(angle));

	r = baseradius;
	r2 = endradius;

	x = m_d.m_Center.x;
	y = m_d.m_Center.y;
	x2 = pvEndCenter->x;
	y2 = pvEndCenter->y;

	// Use quadratic equation to calculate slope of tangent lines.

	a = (r*r  - 2*r2*r + r2*r2  - x2*x2  + 2*x*x2 - x*x);
	b = 2*( y - y2 )*( x - x2 );
	c = (r*r  - 2*r2*r + r2*r2  - y2*y2  + 2*y*y2 - y*y);

	double result, ans1, ans2;
	result = (b*b) - (4*a*c);
	double newangle, na2;

	if (result >= 0)
		{
		ans1 = (-b + (sqrt(result))) / (2*a);
		ans2 = (-b - (sqrt(result))) / (2*a);

		newangle = atan(ans1);
		na2 = atan(ans2);

		if ((y2 > y))
			{
			// Test left edge of outer circle to inner circle, to get pos/neg of tangent
			// Have to do this because atan doesn't diffierentiate between quandrants
			if ((x2-r2 < x-r) ^ (r < r2))
				{
				newangle = PI+newangle;
				}
			if ((x2+r2 < x+r) ^ (r < r2))
				{
				na2 = PI+na2;
				}
			}
		else
			{
			if ((x2+r2 < x+r) ^ (r < r2))
				{
				newangle = PI+newangle;
				}
			if ((x2-r2 < x-r) ^ (r < r2))
				{
				na2 = PI+na2;
				}
			}
		}

	rgvTangents[0].x = (float)(m_d.m_Center.x + baseradius*sin(na2));
	rgvTangents[0].y = (float)(m_d.m_Center.y - baseradius*cos(na2));
	rgvTangents[1].x = (float)(pvEndCenter->x + endradius*sin(na2));
	rgvTangents[1].y = (float)(pvEndCenter->y - endradius*cos(na2));

	rgvTangents[3].x = (float)(m_d.m_Center.x - baseradius*sin(newangle));
	rgvTangents[3].y = (float)(m_d.m_Center.y + baseradius*cos(newangle));
	rgvTangents[2].x = (float)(pvEndCenter->x - endradius*sin(newangle));
	rgvTangents[2].y = (float)(pvEndCenter->y + endradius*cos(newangle));
	}

void Flipper::PreRender(Sur *psur)
	{
	float anglerad = ANGTORAD(m_d.m_StartAngle);
	float anglerad2 = ANGTORAD(m_d.m_EndAngle);

	Vertex vendcenter;
	Vertex rgv[4];

	SetVertices(anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

	psur->SetFillColor(RGB(192,192,192));
	psur->SetBorderColor(-1,fFalse,0);
	psur->SetLineColor(RGB(0,0,0), fFalse, 0);

	psur->SetObject(this);

	psur->Polygon(rgv, 4);
	psur->Ellipse(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius);
	psur->Ellipse(vendcenter.x, vendcenter.y, m_d.m_EndRadius);
	}

void Flipper::Render(Sur *psur)
	{
	float anglerad = ANGTORAD(m_d.m_StartAngle);
	float anglerad2 = ANGTORAD(m_d.m_EndAngle);

	Vertex vendcenter;
	Vertex rgv[4];

	SetVertices(anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

	psur->SetFillColor(RGB(192,192,192));
	psur->SetBorderColor(-1,fFalse,0);
	psur->SetLineColor(RGB(0,0,0), fFalse, 0);

	psur->SetObject(this);

	//psur->Polygon(rgv, 4);
	//psur->Ellipse(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius);
	//psur->Ellipse(vendcenter.x, vendcenter.y, m_d.m_EndRadius);

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
	psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

	psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
	psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

	SetVertices(anglerad2, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

	psur->SetLineColor(RGB(128,128,128), fTrue, 0);

	//psur->Polygon(rgv, 4);
	//psur->Ellipse(m_Center.x, m_Center.y, m_BaseRadius);
	//psur->Ellipse(vendcenter.x, vendcenter.y, m_EndRadius);

	psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
	psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

	psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
	psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

	rgv[0].x = m_d.m_Center.x + (((float)sin(anglerad)) * (m_d.m_FlipperRadius+m_d.m_EndRadius));
	rgv[0].y = m_d.m_Center.y - (((float)cos(anglerad)) * (m_d.m_FlipperRadius+m_d.m_EndRadius));

	rgv[1].x = m_d.m_Center.x + (((float)sin(anglerad2)) * (m_d.m_FlipperRadius+m_d.m_EndRadius));
	rgv[1].y = m_d.m_Center.y - (((float)cos(anglerad2)) * (m_d.m_FlipperRadius+m_d.m_EndRadius));

	if (m_d.m_EndAngle < m_d.m_StartAngle)
		{
		psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius+m_d.m_EndRadius, rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
		}
	else
		{
		psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius+m_d.m_EndRadius, rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);
		}

	//psur->Ellipse2(m_Center.x, m_Center.y, 20);
	}

void Flipper::RenderShadow(ShadowSur *psur, float height)
	{
	/*float anglerad = ANGTORAD(m_d.m_StartAngle);
	float anglerad2 = ANGTORAD(m_d.m_EndAngle);

	Vertex vendcenter;
	Vertex rgv[4];

	SetVertices(anglerad, &vendcenter, rgv);

	psur->SetFillColor(RGB(0,0,0));
	psur->SetBorderColor(-1,fFalse,0);
	psur->SetLineColor(RGB(0,0,0), fFalse, 0);

	//psur->SetObject(this);

	psur->Polygon(rgv, 4);
	psur->Ellipse(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius);
	psur->Ellipse(vendcenter.x, vendcenter.y, m_d.m_EndRadius);*/
	}

void Flipper::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_Center.x, m_d.m_Center.y);
	}

void Flipper::MoveOffset(float dx, float dy)
	{
	m_d.m_Center.x += dx;
	m_d.m_Center.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Flipper::GetCenter(Vertex *pv)
	{
	pv->x = m_d.m_Center.x;
	pv->y = m_d.m_Center.y;
	}

void Flipper::PutCenter(Vertex *pv)
	{
	m_d.m_Center.x = pv->x;
	m_d.m_Center.y = pv->y;

	m_ptable->SetDirtyDraw();
	}

STDMETHODIMP Flipper::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_IFlipper,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP Flipper::RotateToEnd()
{
	if (m_phitflipper)
		{
		if (fOldPhys)
			{
			m_phitflipper->m_flipperanim.m_angleEnd = ANGTORAD(m_d.m_EndAngle);
			m_phitflipper->m_flipperanim.m_anglespeed = 0;
			m_phitflipper->m_flipperanim.m_fAcc = fTrue;
			m_phitflipper->m_flipperanim.m_force = m_d.m_force;
			m_phitflipper->m_forcemass = m_d.m_strength;
			m_phitflipper->m_flipperanim.m_maxvelocity = m_d.m_force * 4.5f;
			//m_phitflipper->m_force = 0.045f;
			//m_phitflipper->m_force = 0.004f;
			}
		else //new phys
			{
			float endAng = ANGTORAD(m_d.m_EndAngle);
			//m_phitflipper->m_flipperanim.m_EnableRotateEvent = 1;
			m_phitflipper->m_flipperanim.m_angleEnd = endAng;

			if (fabsf(endAng - (float)m_phitflipper->m_flipperanim.m_angleCur)< 1.0e-5f)   //already there?
			{
				m_phitflipper->m_flipperanim.m_fAcc = 0;
				m_phitflipper->m_flipperanim.m_anglespeed = 0;
			}
			else { m_phitflipper->m_flipperanim.m_fAcc = (endAng > m_phitflipper->m_flipperanim.m_angleCur)? +1:-1;}

			m_phitflipper->m_flipperanim.m_maxvelocity = m_d.m_force * 4.5f;
			m_phitflipper->m_flipperanim.m_force = m_d.m_force;
			m_phitflipper->m_forcemass = m_d.m_strength;
			}
		}

/*#ifdef LOG
	if (g_pplayer)
		{
		int i = g_pplayer->m_vmover.IndexOf(m_phitflipper);
		fprintf(g_pplayer->m_flog, "Flipper End %d\n", i);
		}
#endif*/

	return S_OK;
}

STDMETHODIMP Flipper::RotateToStart()
{
	if (m_phitflipper)
		{
		if (fOldPhys)
			{
			m_phitflipper->m_flipperanim.m_angleEnd = ANGTORAD(m_d.m_StartAngle);
			m_phitflipper->m_flipperanim.m_anglespeed = 0;
			m_phitflipper->m_flipperanim.m_fAcc = fTrue;
			m_phitflipper->m_flipperanim.m_force = m_d.m_force;
			m_phitflipper->m_forcemass = m_d.m_strength;
			m_phitflipper->m_flipperanim.m_maxvelocity = m_d.m_force * 4.5f;
			//m_phitflipper->m_force = 0.045f;
			//m_phitflipper->m_force = 0.004f;
			}
		else // new phys
			{
			float startAng =  ANGTORAD(m_d.m_StartAngle);		
			//m_phitflipper->m_flipperanim.m_EnableRotateEvent = -1;
			m_phitflipper->m_flipperanim.m_angleEnd = startAng;

			if (fabsf(startAng - (float)m_phitflipper->m_flipperanim.m_angleCur)< 1.0e-5f)//already there?
			{
				m_phitflipper->m_flipperanim.m_fAcc = 0;
				m_phitflipper->m_flipperanim.m_anglespeed = 0;
			}
			else { m_phitflipper->m_flipperanim.m_fAcc = (startAng > m_phitflipper->m_flipperanim.m_angleCur)? +1:-1;}

			// Eliminate "Return Strength" and make the RotateToStart behave similar to RotateToEnd.
			m_phitflipper->m_flipperanim.m_maxvelocity = m_d.m_returnforce * 4.5f;
			m_phitflipper->m_flipperanim.m_force = m_d.m_returnforce;
			m_phitflipper->m_forcemass = m_d.m_returnstrength;
			}
		}

/*#ifdef LOG
	if (g_pplayer)
		{
		int i = g_pplayer->m_vmover.IndexOf(m_phitflipper);
		fprintf(g_pplayer->m_flog, "Flipper Start %d\n", i);
		}
#endif*/

	return S_OK;
}

void Flipper::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

void Flipper::RenderAtThickness(LPDIRECT3DDEVICE7 pd3dDevice, ObjFrame *pof, float angle,  float height, COLORREF color, float baseradius, float endradius, float flipperheight)
	{
	Pin3D *ppin3d = &g_pplayer->m_pin3d;

	float r = (color & 255) / 255.0f;
	float g = (color & 65280) / 65280.0f;
	float b = (color & 16711680) / 16711680.0f;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = r;
	mtrl.diffuse.g = mtrl.ambient.g = g;
	mtrl.diffuse.b = mtrl.ambient.b = b;
	mtrl.diffuse.a = mtrl.ambient.a = 1.0;
	pd3dDevice->SetMaterial(&mtrl);

	Vertex vendcenter;
	Vertex rgv[4];
	WORD rgi[8];
	WORD rgiNormal[3];

	Vertex3D rgv3D[32];

	SetVertices(angle, &vendcenter, rgv, baseradius, endradius);

	int l;
	for (l=0;l<8;l++)
		{
		rgv3D[l].x = rgv[l&3].x;
		rgv3D[l].y = rgv[l&3].y;
		if (l<4)
			{
			rgv3D[l].z = height + flipperheight + 0.15f; // Make flippers a bit taller so they draw above walls
			}
		else
			{
			rgv3D[l].z = height + 0;
			}
		rgi[l] = l;

		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
		}

	ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitflipper->m_flipperanim.m_znear, &m_phitflipper->m_flipperanim.m_zfar, 8, fFalse);

	SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 8,
												  rgi, 4, NULL);

	rgi[0] = 0;
	rgi[1] = 4;
	rgi[2] = 5;
	rgi[3] = 1;

	SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 8,
												  rgi, 4, NULL);

	rgi[0] = 2;
	rgi[1] = 6;
	rgi[2] = 7;
	rgi[3] = 3;

	SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 8,
												  rgi, 4, NULL);



	// Base circle
	for (l=0;l<16;l++)
		{
		float angle = PI*2;
		angle /= 16;
		angle *= l;
		rgv3D[l].x = (float)sin(angle)*baseradius + m_d.m_Center.x;
		rgv3D[l].y = (float)-cos(angle)*baseradius + m_d.m_Center.y;
		rgv3D[l].z = height + flipperheight + 0.1f;
		rgv3D[l+16].x = rgv3D[l].x;
		rgv3D[l+16].y = rgv3D[l].y;
		rgv3D[l+16].z = height + 0;

		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l+16]);
		}

	ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitflipper->m_flipperanim.m_znear, &m_phitflipper->m_flipperanim.m_zfar, 32, fFalse);

	for (l=1;l<15;l++)
		{
		rgi[0] = 0;
		rgi[1] = l;
		rgi[2] = l+1;

		SetNormal(rgv3D, rgi, 3, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 32,
												  rgi, 3, NULL);


		rgiNormal[0] = (l+14) % 16;
		rgiNormal[1] = rgiNormal[0] + 16;
		rgiNormal[2] = l;

		rgi[0] = l-1;
		rgi[1] = l-1+16;
		rgi[3] = l;
		rgi[2] = l+16;

		SetNormal(rgv3D, rgiNormal, 3, NULL, rgi, 2);

		rgiNormal[0] = l-1;
		rgiNormal[1] = l-1+16;
		rgiNormal[2] = (l+1) % 16;

		SetNormal(rgv3D, rgiNormal, 3, NULL, &rgi[2], 2);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 32,
												  rgi, 4, NULL);
		}

	// End circle
	for (l=0;l<16;l++)
		{
		float angle = PI*2;
		angle /= 16;
		angle *= l;
		rgv3D[l].x = (float)sin(angle)*endradius + vendcenter.x;
		rgv3D[l].y = (float)-cos(angle)*endradius + vendcenter.y;
		rgv3D[l].z = height + flipperheight + 0.1f;
		rgv3D[l+16].x = rgv3D[l].x;
		rgv3D[l+16].y = rgv3D[l].y;
		rgv3D[l+16].z = height + 0;

		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
		ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l+16]);
		}

	ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitflipper->m_flipperanim.m_znear, &m_phitflipper->m_flipperanim.m_zfar, 32, fFalse);

	for (l=1;l<15;l++)
		{
		rgi[0] = 0;
		rgi[1] = l;
		rgi[2] = l+1;

		SetNormal(rgv3D, rgi, 3, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 32,
												  rgi, 3, NULL);

		rgiNormal[0] = (l+14) % 16;
		rgiNormal[1] = rgiNormal[0] + 16;
		rgiNormal[2] = l;

		rgi[0] = l-1;
		rgi[1] = l-1+16;
		rgi[3] = l;
		rgi[2] = l+16;

		SetNormal(rgv3D, rgiNormal, 3, NULL, rgi, 2);

		rgiNormal[0] = l-1;
		rgiNormal[1] = l-1+16;
		rgiNormal[2] = (l+1) % 16;

		SetNormal(rgv3D, rgiNormal, 3, NULL, &rgi[2], 2);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 32,
												  rgi, 4, NULL);
		}
	}
	
void Flipper::RenderMoversFromCache(Pin3D *ppin3d)
	{
	ppin3d->ReadAnimObjectFromCacheFile(&m_phitflipper->m_flipperanim, &m_phitflipper->m_flipperanim.m_vddsFrame);
	}

void Flipper::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	_ASSERTE(m_phitflipper);
	Pin3D *ppin3d = &g_pplayer->m_pin3d;
	LPDIRECTDRAWSURFACE7 pdds;
	//DDBLTFX ddbfx;
	ObjFrame *pof;

	float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

	ppin3d->ClearExtents(&m_phitflipper->m_flipperanim.m_rcBounds, &m_phitflipper->m_flipperanim.m_znear, &m_phitflipper->m_flipperanim.m_zfar);

	float anglerad = ANGTORAD(m_d.m_StartAngle);
	float anglerad2 = ANGTORAD(m_d.m_EndAngle);

	ppin3d->SetTexture(NULL);

	int cframes = abs(((int)(m_d.m_EndAngle - m_d.m_StartAngle)) / 6);//10;
	cframes = max(cframes,2);

	int i;

	for (i=0;i<cframes;i++)
		{
		float angle = anglerad + ((anglerad2 - anglerad)*i/(cframes-1));

		pof = new ObjFrame();

		ppin3d->ClearExtents(&pof->rc, NULL, NULL);

		RenderAtThickness(pd3dDevice, pof, angle,  height, m_d.m_color, m_d.m_BaseRadius - m_d.m_rubberthickness, m_d.m_EndRadius - m_d.m_rubberthickness, 50);

		if (m_d.m_rubberthickness > 0)
			{
			RenderAtThickness(pd3dDevice, pof, angle,  height + 8, m_d.m_rubbercolor, m_d.m_BaseRadius, m_d.m_EndRadius, 34);
			}

		pdds = ppin3d->CreateOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
		pof->pddsZBuffer = ppin3d->CreateZBufferOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);

		pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &pof->rc, 0, NULL);
		//HRESULT hr = pof->pddsZBuffer->Blt(NULL, ppin3d->m_pddsZBuffer, &pof->rc, 0, NULL);
		HRESULT hr = pof->pddsZBuffer->BltFast(0, 0, ppin3d->m_pddsZBuffer, &pof->rc, DDBLTFAST_NOCOLORKEY);
		
		//pdds->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbfx);
		m_phitflipper->m_flipperanim.m_vddsFrame.AddElement(pof);
		pof->pdds = pdds;

		ppin3d->ExpandRectByRect(&m_phitflipper->m_flipperanim.m_rcBounds, &pof->rc);

		// reset the portion of the z-buffer that we changed
		ppin3d->m_pddsZBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY);
		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;
		ppin3d->m_pddsBackBuffer->Blt(&pof->rc, NULL,
				&pof->rc, DDBLT_COLORFILL, &ddbltfx);
		}

	ppin3d->WriteAnimObjectToCacheFile(&m_phitflipper->m_flipperanim, &m_phitflipper->m_flipperanim.m_vddsFrame);
	/*float r = (m_d.m_color & 255) / 255.0f;
	float g = (m_d.m_color & 65280) / 65280.0f;
	float b = (m_d.m_color & 16711680) / 16711680.0f;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = r;
	mtrl.diffuse.g = mtrl.ambient.g = g;
	mtrl.diffuse.b = mtrl.ambient.b = b;
	mtrl.diffuse.a = mtrl.ambient.a = 1.0;
	pd3dDevice->SetMaterial(&mtrl);

	int cframes = 10;

	int i;

	//ddbfx.dwSize = sizeof(DDBLTFX);
	//ddbfx.dwFillColor = RGB(0,0,0);

	float anglerad = ANGTORAD(m_d.m_StartAngle);
	float anglerad2 = ANGTORAD(m_d.m_EndAngle);

	Vertex vendcenter;
	Vertex rgv[4];
	WORD rgi[8];
	WORD rgiNormal[3];

	Vertex3D rgv3D[32];



	ppin3d->ClearExtents(&m_phitflipper->m_rcBounds, &m_phitflipper->m_znear, &m_phitflipper->m_zfar);

	for (i=0;i<cframes;i++)
		{
		//pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET ,
					   //0x00000000, 1.0f, 0L );

		pof = new ObjFrame();

		SetVertices(anglerad + ((anglerad2 - anglerad)*i/(cframes-1)), &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

		//ppin3d->m_pddsBackBuffer->Blt(NULL, ppin3d->m_pddsStatic, NULL, 0, NULL);
		//ppin3d->m_pddsZBuffer->Blt(NULL, ppin3d->m_pddsStaticZ, NULL, 0, NULL);

		int l;
		for (l=0;l<8;l++)
			{
			rgv3D[l].x = rgv[l&3].x;
			rgv3D[l].y = rgv[l&3].y;
			if (l<4)
				{
				rgv3D[l].z = height + 50.15f; // Make flippers a bit taller so they draw above walls
				}
			else
				{
				rgv3D[l].z = height + 0;
				}
			rgi[l] = l;

			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
			}

		ppin3d->ClearExtents(&pof->rc, NULL, NULL);
		ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitflipper->m_znear, &m_phitflipper->m_zfar, 8);

		SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 4, NULL);

		rgi[0] = 0;
		rgi[1] = 4;
		rgi[2] = 5;
		rgi[3] = 1;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 4, NULL);

		rgi[0] = 2;
		rgi[1] = 6;
		rgi[2] = 7;
		rgi[3] = 3;

		SetNormal(rgv3D, rgi, 4, NULL, NULL, 0);

		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 8,
													  rgi, 4, NULL);



		// Base circle
		for (l=0;l<16;l++)
			{
			float angle = PI*2;
			angle /= 16;
			angle *= l;
			rgv3D[l].x = (float)sin(angle)*m_d.m_BaseRadius + m_d.m_Center.x;
			rgv3D[l].y = (float)-cos(angle)*m_d.m_BaseRadius + m_d.m_Center.y;
			rgv3D[l].z = height + 50.1f;
			rgv3D[l+16].x = rgv3D[l].x;
			rgv3D[l+16].y = rgv3D[l].y;
			rgv3D[l+16].z = height + 0;

			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l+16]);
			}

		ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitflipper->m_znear, &m_phitflipper->m_zfar, 32);

		for (l=1;l<15;l++)
			{
			rgi[0] = 0;
			rgi[1] = l;
			rgi[2] = l+1;

			SetNormal(rgv3D, rgi, 3, NULL, NULL, 0);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 32,
													  rgi, 3, NULL);


			rgiNormal[0] = (l+14) % 16;
			rgiNormal[1] = rgiNormal[0] + 16;
			rgiNormal[2] = l;

			rgi[0] = l-1;
			rgi[1] = l-1+16;
			rgi[3] = l;
			rgi[2] = l+16;

			SetNormal(rgv3D, rgiNormal, 3, NULL, rgi, 2);

			rgiNormal[0] = l-1;
			rgiNormal[1] = l-1+16;
			rgiNormal[2] = (l+1) % 16;

			SetNormal(rgv3D, rgiNormal, 3, NULL, &rgi[2], 2);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 32,
													  rgi, 4, NULL);
			}

		// End circle
		for (l=0;l<16;l++)
			{
			float angle = PI*2;
			angle /= 16;
			angle *= l;
			rgv3D[l].x = (float)sin(angle)*m_d.m_EndRadius + vendcenter.x;
			rgv3D[l].y = (float)-cos(angle)*m_d.m_EndRadius + vendcenter.y;
			rgv3D[l].z = height + 50.1f;
			rgv3D[l+16].x = rgv3D[l].x;
			rgv3D[l+16].y = rgv3D[l].y;
			rgv3D[l+16].z = height + 0;

			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
			ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l+16]);
			}

		ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitflipper->m_znear, &m_phitflipper->m_zfar, 32);

		for (l=1;l<15;l++)
			{
			rgi[0] = 0;
			rgi[1] = l;
			rgi[2] = l+1;

			SetNormal(rgv3D, rgi, 3, NULL, NULL, 0);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 32,
													  rgi, 3, NULL);

			rgiNormal[0] = (l+14) % 16;
			rgiNormal[1] = rgiNormal[0] + 16;
			rgiNormal[2] = l;

			rgi[0] = l-1;
			rgi[1] = l-1+16;
			rgi[3] = l;
			rgi[2] = l+16;

			SetNormal(rgv3D, rgiNormal, 3, NULL, rgi, 2);

			rgiNormal[0] = l-1;
			rgiNormal[1] = l-1+16;
			rgiNormal[2] = (l+1) % 16;

			SetNormal(rgv3D, rgiNormal, 3, NULL, &rgi[2], 2);

			pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
													  rgv3D, 32,
													  rgi, 4, NULL);
			}*/

		/*pdds = ppin3d->CreateOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);
		pof->pddsZBuffer = ppin3d->CreateZBufferOffscreen(pof->rc.right - pof->rc.left, pof->rc.bottom - pof->rc.top);

		pdds->Blt(NULL, ppin3d->m_pddsBackBuffer, &pof->rc, 0, NULL);
		//HRESULT hr = pof->pddsZBuffer->Blt(NULL, ppin3d->m_pddsZBuffer, &pof->rc, 0, NULL);
		HRESULT hr = pof->pddsZBuffer->BltFast(0, 0, ppin3d->m_pddsZBuffer, &pof->rc, DDBLTFAST_NOCOLORKEY);

		//pdds->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddbfx);
		m_phitflipper->m_vddsFrame.AddElement(pof);
		pof->pdds = pdds;

		ppin3d->ExpandRectByRect(&m_phitflipper->m_rcBounds, &pof->rc);

		// reset the portion of the z-buffer that we changed
		ppin3d->m_pddsZBuffer->BltFast(pof->rc.left, pof->rc.top, ppin3d->m_pddsStaticZ, &pof->rc, DDBLTFAST_NOCOLORKEY);
		// Reset color key in back buffer
		DDBLTFX ddbltfx;
		ddbltfx.dwSize = sizeof(DDBLTFX);
		ddbltfx.dwFillColor = 0;
		ppin3d->m_pddsBackBuffer->Blt(&pof->rc, NULL,
				&pof->rc, DDBLT_COLORFILL, &ddbltfx);
		}*/
	}

/*STDMETHODIMP Flipper::get_Application(IVisualPinball **lppaReturn)
	{
	return g_pvp->QueryInterface(IID_IVisualPinball, (void **)lppaReturn);
	}

STDMETHODIMP Flipper::get_Parent(ITable **lppaReturn)
	{
	return m_ptable->QueryInterface(IID_ITable, (void **)lppaReturn);
	}*/

HRESULT Flipper::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_Center, sizeof(Vertex));
	bw.WriteFloat(FID(BASR), m_d.m_BaseRadius);
	bw.WriteFloat(FID(ENDR), m_d.m_EndRadius);
	bw.WriteFloat(FID(FLPR), m_d.m_FlipperRadius);
	bw.WriteFloat(FID(ANGS), m_d.m_StartAngle);
	bw.WriteFloat(FID(ANGE), m_d.m_EndAngle);
	bw.WriteFloat(FID(FORC), m_d.m_force);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteString(FID(SURF), m_d.m_szSurface);
	bw.WriteInt(FID(COLR), m_d.m_color);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
	bw.WriteInt(FID(FCLR), m_d.m_rubbercolor);
	bw.WriteInt(FID(RTHK), m_d.m_rubberthickness);
	bw.WriteFloat(FID(STRG), m_d.m_strength);
	bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
	bw.WriteBool(FID(VSBL), m_d.m_fVisible);

// Begin New Physics
	//bw.WriteFloat(FID(FLPR), m_d.m_FlipperRadiusMax);
	bw.WriteFloat(FID(FCOIL), m_d.m_recoil);
	bw.WriteFloat(FID(FAEOS), m_d.m_angleEOS);
	bw.WriteFloat(FID(FRTN), m_d.m_return);
	bw.WriteFloat(FID(RFRC), m_d.m_returnforce);
	bw.WriteInt(FID(RHGT), m_d.m_rubberheight);
	bw.WriteInt(FID(RWDT), m_d.m_rubberwidth);
	bw.WriteFloat(FID(RSGT), m_d.m_returnstrength);
	bw.WriteFloat(FID(FPWL), m_d.m_powerlaw);	
	bw.WriteFloat(FID(FOCB), m_d.m_baseobliquecorrection);	
	bw.WriteFloat(FID(FOCR), m_d.m_obliquecorrection);	
	bw.WriteFloat(FID(FOCT), m_d.m_tipobliquecorrection);	
	bw.WriteFloat(FID(FSCT), m_d.m_scatterangle);	
	bw.WriteFloat(FID(FRMN), m_d.m_FlipperRadiusMin);		
	bw.WriteFloat(FID(FHGT), m_d.m_height);

// End New Physics

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	/*ULONG writ = 0;
	HRESULT hr = S_OK;

	DWORD dwID = ApcProjectItem.ID();
	if(FAILED(hr = pstm->Write(&dwID, sizeof dwID, &writ)))
		return hr;

	if(FAILED(hr = pstm->Write(&m_d, sizeof(FlipperData), &writ)))
		return hr;

	return hr;*/
	}

HRESULT Flipper::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
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

	if(FAILED(hr = pstm->Read(&m_d, sizeof(FlipperData), &read)))
		return hr;

	//ApcProjectItem.Register(ptable->ApcProject, GetDispatch(), dwID);
	*pid = dwID;

	return hr;
#endif
	}

BOOL Flipper::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VCEN))
		{
		pbr->GetStruct(&m_d.m_Center, sizeof(Vertex));
		}
	else if (id == FID(BASR))
		{
		pbr->GetFloat(&m_d.m_BaseRadius);
		}
	else if (id == FID(ENDR))
		{
		pbr->GetFloat(&m_d.m_EndRadius);
		}
	else if (id == FID(FLPR))
		{
		pbr->GetFloat(&m_d.m_FlipperRadius);
		}
	else if (id == FID(ANGS))
		{
		pbr->GetFloat(&m_d.m_StartAngle);
		}
	else if (id == FID(ANGE))
		{
		pbr->GetFloat(&m_d.m_EndAngle);
		}
	else if (id == FID(FORC))
		{
		pbr->GetFloat(&m_d.m_force);
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
	else if (id == FID(COLR))
		{
		pbr->GetInt(&m_d.m_color);
		}
	else if (id == FID(FCLR))
		{
		pbr->GetInt(&m_d.m_rubbercolor);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(RTHK))
		{
		pbr->GetInt(&m_d.m_rubberthickness);
		}
	else if (id == FID(STRG))
		{
		pbr->GetFloat(&m_d.m_strength);
		}
	else if (id == FID(ELAS))
		{
		pbr->GetFloat(&m_d.m_elasticity);
		}
	else if (id == FID(VSBL))
		{
		pbr->GetBool(&m_d.m_fVisible);
		}
// Begin New Physics
	else if (id == FID(FCOIL))
		{
		pbr->GetFloat(&m_d.m_recoil);
		}
	else if (id == FID(FAEOS))
		{
		pbr->GetFloat(&m_d.m_angleEOS);
		}
	else if (id == FID(FRTN))
		{
		pbr->GetFloat(&m_d.m_return);
		}
	else if (id == FID(RFRC))
		{
		pbr->GetFloat(&m_d.m_returnforce);
		}
	else if (id == FID(RHGT))
		{
		pbr->GetInt(&m_d.m_rubberheight);
		}
	else if (id == FID(RWDT))
		{
		pbr->GetInt(&m_d.m_rubberwidth);
		}
	else if (id == FID(FHGT))
		{
		pbr->GetFloat(&m_d.m_height);
		}
	else if (id == FID(RSGT))
		{
		pbr->GetFloat(&m_d.m_returnstrength);
		}
	else if (id == FID(FPWL))
		{
		pbr->GetFloat(&m_d.m_powerlaw);
		}
	else if (id == FID(FOCR))
		{
		pbr->GetFloat(&m_d.m_obliquecorrection); 

		// Check if base and tip oblique corrections have no values.
		if ( (m_d.m_baseobliquecorrection == 0.0f) && (m_d.m_tipobliquecorrection == 0.0f) )
			{
			// Initialize base and tip with the center value.
			// This is done for backward compatibility.
			m_d.m_baseobliquecorrection = m_d.m_obliquecorrection;
			m_d.m_tipobliquecorrection = m_d.m_obliquecorrection;
			}
		}
	else if (id == FID(FOCB))
		{
		pbr->GetFloat(&m_d.m_baseobliquecorrection); 
		}
	else if (id == FID(FOCT))
		{
		pbr->GetFloat(&m_d.m_tipobliquecorrection); 
		}
	else if (id == FID(FSCT))
		{
		pbr->GetFloat(&m_d.m_scatterangle);
		}
	else if (id == FID(FRMN))
		{
		pbr->GetFloat(&m_d.m_FlipperRadiusMin);	
		}
// End New Physics
	else
		{
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT Flipper::InitPostLoad()
	{
	return S_OK;
	}

STDMETHODIMP Flipper::get_BaseRadius(float *pVal)
{
	*pVal = m_d.m_BaseRadius;

	return S_OK;
}

STDMETHODIMP Flipper::put_BaseRadius(float newVal)
{
	STARTUNDO

	m_d.m_BaseRadius = newVal;

	STOPUNDO;

	return S_OK;
}

STDMETHODIMP Flipper::get_EndRadius(float *pVal)
{
	*pVal = m_d.m_EndRadius;

	return S_OK;
}

STDMETHODIMP Flipper::put_EndRadius(float newVal)
{
	STARTUNDO

	m_d.m_EndRadius = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_Length(float *pVal)
{
	*pVal = m_d.m_FlipperRadius;

	return S_OK;
}

STDMETHODIMP Flipper::put_Length(float newVal)
{
	STARTUNDO

	m_d.m_FlipperRadius = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_StartAngle(float *pVal)
{
	*pVal = m_d.m_StartAngle;

	return S_OK;
}

STDMETHODIMP Flipper::put_StartAngle(float newVal)
{
	STARTUNDO

	m_d.m_StartAngle = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_EndAngle(float *pVal)
{
	*pVal = m_d.m_EndAngle;

	return S_OK;
}

STDMETHODIMP Flipper::put_EndAngle(float newVal)
{
	STARTUNDO

	m_d.m_EndAngle = newVal;

	STOPUNDO;

	return S_OK;
}

STDMETHODIMP Flipper::get_CurrentAngle(float *pVal)
{
	if (m_phitflipper)
		{
		*pVal = (float)(m_phitflipper->m_flipperanim.m_angleCur * 360.0f / (2*PI));
		return S_OK;
		}
	else
		{
		return E_FAIL;
		}
}


STDMETHODIMP Flipper::get_X(float *pVal)
{
	*pVal = m_d.m_Center.x;

	return S_OK;
}

STDMETHODIMP Flipper::put_X(float newVal)
{
	STARTUNDO

	m_d.m_Center.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_Y(float *pVal)
{
	*pVal = m_d.m_Center.y;

	return S_OK;
}

STDMETHODIMP Flipper::put_Y(float newVal)
{
	STARTUNDO

	m_d.m_Center.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_Surface(BSTR *pVal)
{
	OLECHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Flipper::put_Surface(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_Color(OLE_COLOR *pVal)
{
	*pVal = m_d.m_color;

	return S_OK;
}

STDMETHODIMP Flipper::put_Color(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_color = newVal;

	STOPUNDO

	return S_OK;
}

/*HRESULT Flipper::GetTypeName(BSTR *pVal)
	{
	*pVal = SysAllocString(L"Flipper");

	return S_OK;
	}*/

/*int Flipper::GetDialogID()
	{
	return IDD_PROPFLIPPER;
	}*/

void Flipper::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPFLIPPER_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPFLIPPER_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPFLIPPER_PHYSICS, IDS_PHYSICS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}

STDMETHODIMP Flipper::get_Speed(float *pVal)
{
	*pVal = m_d.m_force;

	return S_OK;
}

STDMETHODIMP Flipper::put_Speed(float newVal)
{
	STARTUNDO

	m_d.m_force = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_RubberColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_rubbercolor;

	return S_OK;
}

STDMETHODIMP Flipper::put_RubberColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_rubbercolor = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_RubberThickness(long *pVal)
{
	*pVal = m_d.m_rubberthickness;

	return S_OK;
}

STDMETHODIMP Flipper::put_RubberThickness(long newVal)
{
	STARTUNDO

	m_d.m_rubberthickness = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_Strength(float *pVal)
{
	*pVal = m_d.m_strength;

	return S_OK;
}

STDMETHODIMP Flipper::put_Strength(float newVal)
{
	STARTUNDO

	m_d.m_strength = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Flipper::get_Visible(VARIANT_BOOL *pVal)
{
	*pVal = FTOVB(m_d.m_fVisible);

	return S_OK;
}

STDMETHODIMP Flipper::put_Visible(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_fVisible = VBTOF(newVal);

	STOPUNDO

	if (m_phitflipper)
		{
		m_phitflipper->m_flipperanim.m_fEnabled = m_d.m_fVisible;
		}

	return S_OK;
}

STDMETHODIMP Flipper::get_Elasticity(float *pVal)
{
	*pVal = m_d.m_elasticity;

	return S_OK;
}

STDMETHODIMP Flipper::put_Elasticity(float newVal)
{
	STARTUNDO

	m_d.m_elasticity = newVal;

	STOPUNDO

	if (m_phitflipper)
		{
		m_phitflipper->m_elasticity = m_d.m_elasticity;
		}

	return S_OK;
}

STDMETHODIMP Flipper::get_Height(float *pVal)
{
	*pVal = m_d.m_height;

	return S_OK;
}

STDMETHODIMP Flipper::put_Height(float newVal)
{
	STARTUNDO

		m_d.m_height = newVal;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::get_Mass(float *pVal)
{
	*pVal = m_d.m_mass;

	return S_OK;
}

STDMETHODIMP Flipper::put_Mass(float newVal)
{
	STARTUNDO

		m_d.m_mass = newVal;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::get_Return(float *pVal)
{
	*pVal = m_d.m_return;

	return S_OK;
}

STDMETHODIMP Flipper::put_Return(float newVal)
{
	STARTUNDO

		if (newVal < 0) newVal = 0;
		else if (newVal > 1.0f) newVal = 1.0f;

		m_d.m_return = newVal;

		STOPUNDO

			return S_OK;
}

STDMETHODIMP Flipper::get_PowerLaw(float *pVal)
{
	*pVal = m_d.m_powerlaw;

	return S_OK;
}

STDMETHODIMP Flipper::put_PowerLaw(float newVal)
{
	STARTUNDO

		if (newVal < 0) newVal = 0;
		else if (newVal > 4) newVal = 4;

		m_d.m_powerlaw = newVal;

		STOPUNDO

			return S_OK;
}

STDMETHODIMP Flipper::get_ObliqueCorrection(float *pVal)
{
	*pVal = m_d.m_obliquecorrection*180.0f/PI;

	return S_OK;
}

STDMETHODIMP Flipper::get_BaseObliqueCorrection(float *pVal)
{
	*pVal = m_d.m_baseobliquecorrection*180.0f/PI;

	return S_OK;
}

STDMETHODIMP Flipper::get_TipObliqueCorrection(float *pVal)
{
	*pVal = m_d.m_tipobliquecorrection*180.0f/PI;

	return S_OK;
}

STDMETHODIMP Flipper::put_ScatterAngle(float newVal)
{
	STARTUNDO

		m_d.m_scatterangle = ANGTORAD(newVal);

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::get_ScatterAngle(float *pVal)
{
	*pVal = RADTOANG(m_d.m_scatterangle);

	return S_OK;
}

STDMETHODIMP Flipper::put_ObliqueCorrection(float newVal)
{
	STARTUNDO

		m_d.m_obliquecorrection = newVal*PI/180.0f;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::put_BaseObliqueCorrection(float newVal)
{
	STARTUNDO

		m_d.m_baseobliquecorrection = newVal*PI/180.0f;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::put_TipObliqueCorrection(float newVal)
{
	STARTUNDO

		m_d.m_tipobliquecorrection = newVal*PI/180.0f;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::get_FlipperRadiusMin(float *pVal)
{
	*pVal = m_d.m_FlipperRadiusMin;

	return S_OK;
}

STDMETHODIMP Flipper::put_FlipperRadiusMin(float newVal)
{
	STARTUNDO

		if (newVal < 0) newVal = 0;

	m_d.m_FlipperRadiusMin = newVal;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::get_Recoil(float *pVal)
{
	*pVal = m_d.m_recoil;

	return S_OK;
}

STDMETHODIMP Flipper::put_Recoil(float newVal)
{
	STARTUNDO

		m_d.m_recoil = newVal;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::get_AngleEOS(float *pVal)
{
	*pVal = m_d.m_angleEOS;

	return S_OK;
}

STDMETHODIMP Flipper::put_AngleEOS(float newVal)
{
	STARTUNDO

		m_d.m_angleEOS = newVal;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::get_RubberHeight(long *pVal)
{
	*pVal = m_d.m_rubberheight;

	return S_OK;
}

STDMETHODIMP Flipper::get_RubberWidth(long *pVal)
{
	*pVal = m_d.m_rubberwidth;

	return S_OK;
}

STDMETHODIMP Flipper::put_RubberHeight(long newVal)
{
	STARTUNDO

		m_d.m_rubberheight = newVal;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::put_RubberWidth(long newVal)
{
	STARTUNDO

		m_d.m_rubberwidth = newVal;

	STOPUNDO

		return S_OK;
}

STDMETHODIMP Flipper::get_ReturnSpeed(float *pVal)
{
	if (m_phitflipper)
	{
		*pVal = (float)m_phitflipper->m_flipperanim.m_anglespeed * 180.0f /PI;
	}
	else *pVal = m_d.m_returnforce;

	return S_OK;
}

STDMETHODIMP Flipper::put_ReturnSpeed(float newVal)
{
	if (m_phitflipper)
	{
		m_phitflipper->m_flipperanim.m_anglespeed = newVal *PI/180.0f;
	}
	else
	{
		STARTUNDO
			m_d.m_returnforce = newVal;
		STOPUNDO
	}

	return S_OK;
}

STDMETHODIMP Flipper::get_ReturnStrength(float *pVal)
{
	*pVal = m_d.m_returnstrength;

	return S_OK;
}

STDMETHODIMP Flipper::put_ReturnStrength(float newVal)
{
	STARTUNDO

		m_d.m_returnstrength = newVal;

	STOPUNDO

		return S_OK;
}