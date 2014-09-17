#include "StdAfx.h"

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
		m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? fFalse : fTrue;
	else
		m_d.m_tdr.m_fTimerEnabled = fFalse;
	
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

	}

void Kicker::WriteRegDefaults()
	{
	SetRegValue("DefaultProps\\Kicker","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
	SetRegValue("DefaultProps\\Kicker","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
	SetRegValue("DefaultProps\\Kicker","Enabled",REG_DWORD,&m_d.m_fEnabled,4);
	SetRegValueFloat("DefaultProps\\Kicker","HitHeight", m_d.m_hit_height);
	SetRegValueFloat("DefaultProps\\Kicker","Radius", m_d.m_radius);
	SetRegValueFloat("DefaultProps\\Kicker","Scatter", m_d.m_scatter);
	SetRegValue("DefaultProps\\Kicker","KickerType",REG_DWORD,&m_d.m_kickertype,4);
	SetRegValue("DefaultProps\\Kicker","Surface", REG_SZ, &m_d.m_szSurface,lstrlen(m_d.m_szSurface));
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

void Kicker::PostRenderStatic(RenderDevice* pd3dDevice)
{
}

void Kicker::RenderSetup(RenderDevice* pd3dDevice)
{
   if ((m_d.m_kickertype == KickerInvisible) || (m_d.m_kickertype == KickerHidden))
      return;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_zScale;

   for (int l=0;l<16;l++)
   {
      const float angle = (float)(M_PI*2.0/16.0)*(float)l;
      // 0-15 : lower ring
      vertices[l].x = m_d.m_vCenter.x + sinf(angle)*m_d.m_radius;
      vertices[l].y = m_d.m_vCenter.y - cosf(angle)*m_d.m_radius;
      vertices[l].z = height - 30.0f;

      // 16-31 : top ring
      vertices[l+16].x = vertices[l].x;
      vertices[l+16].y = vertices[l].y;
      vertices[l+16].z = height + 0.1f*m_ptable->m_zScale;

      // 32-47 : top outer ring
      vertices[l+32].x = m_d.m_vCenter.x + sinf(angle)*(m_d.m_radius+6.0f);
      vertices[l+32].y = m_d.m_vCenter.y - cosf(angle)*(m_d.m_radius+6.0f);
      vertices[l+32].z = height + 0.05f*m_ptable->m_zScale;
   }

   vertices[48].x = m_d.m_vCenter.x;
   vertices[48].y = m_d.m_vCenter.y;
   vertices[48].z = height + (0.1f - 30.0f)*m_ptable->m_zScale;

   ppin3d->CalcShadowCoordinates(vertices,16*3+1);
}


void Kicker::PreRenderStatic( RenderDevice* pd3dDevice)
{
   // Don't process "invisible" kickers.
   if ((m_d.m_kickertype == KickerInvisible) || (m_d.m_kickertype == KickerHidden))
      return;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexNormalTexelTexelDeclaration );

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);	
   pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);	
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);

   const float inv_width  = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
   const float inv_height = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);

   Material *mat = m_ptable->GetMaterial( m_d.m_szMaterial);
   D3DXVECTOR4 diffuseColor( 0.5f, 0.5f, 0.5f, 1.0f );
   D3DXVECTOR4 glossyColor( 0.04f, 0.04f, 0.04f, 1.0f );
   D3DXVECTOR4 specularColor( 0.04f, 0.04f, 0.04f, 1.0f );
   float diffuseWrap = 0.5f;
   float glossyPower = 0.1f;
   bool  bDiffActive=true;
   bool  bGlossyActive = false;
   bool  bSpecActive = false;
   if( mat )
   {
      diffuseColor = mat->getDiffuseColor();
      glossyColor = mat->getGlossyColor();
      specularColor = mat->getSpecularColor();
      diffuseWrap = mat->m_fDiffuse;
      glossyPower = mat->m_fGlossy;
      bDiffActive = mat->m_bDiffuseActive;
      bGlossyActive = mat->m_bGlossyActive;
      bSpecActive = mat->m_bSpecularActive;
   }

   pd3dDevice->basicShader->Core()->SetFloat("fDiffuseWrap",diffuseWrap);
   pd3dDevice->basicShader->Core()->SetFloat("fGlossyPower",glossyPower);
   pd3dDevice->basicShader->Core()->SetVector("vDiffuseColor",&diffuseColor);
   pd3dDevice->basicShader->Core()->SetVector("vGlossyColor",&glossyColor);
   pd3dDevice->basicShader->Core()->SetVector("vSpecularColor",&specularColor);
   pd3dDevice->basicShader->Core()->SetBool("bDiffuse", bDiffActive);
   pd3dDevice->basicShader->Core()->SetBool("bGlossy", bGlossyActive);
   pd3dDevice->basicShader->Core()->SetBool("bSpecular", bSpecActive);
   pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");

   ppin3d->EnableLightMap(height);

   // Draw the inside of the kicker based on its type.
   switch (m_d.m_kickertype)
   {
      case KickerHole: 
      {
         for (int l=0;l<16;++l)
         {
            WORD rgiNormal[6] = {
               (l-1+16) % 16,
               (l-1+16) % 16 + 16,
               (l-1+16) % 16 + 2,
               l,
               l+16,
               (l+2) % 16 };

            WORD rgi[4] = {
               l,
               l+16,
               (l+1) % 16 + 16,
               (l+1) % 16 };

            SetNormal(vertices, rgiNormal, 3, NULL, rgi, 2);
            SetNormal(vertices, &rgiNormal[3], 3, NULL, &rgi[2], 2);
            pd3dDevice->basicShader->Begin(0);
            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,vertices, 32, rgi, 4);
            pd3dDevice->basicShader->End();
         }

         diffuseColor = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
         pd3dDevice->basicShader->Core()->SetVector("vDiffuseColor",&diffuseColor);
         // Draw the bottom of the kicker hole
         WORD rgi[3*14];
         for (int l=0;l<14;++l)
         {
            rgi[l*3  ] = 0;
            rgi[l*3+1] = l+1;
            rgi[l*3+2] = l+2;

            SetNormal(vertices, rgi+l*3, 3, NULL, NULL, 0);
         }
         pd3dDevice->basicShader->Begin(0);
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX,vertices, 16, rgi, 3*14);
         pd3dDevice->basicShader->End();

         // Draw the top "lid" of the kicker hole invisibly (for the depth buffer)
         for (int l=0;l<14;++l)
         {
            SetNormal(vertices+16, rgi+l*3, 3, NULL, NULL, 0);
         }
         float depthbias = -1e-4f;
         pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, *((DWORD*)&depthbias));
         pd3dDevice->SetRenderState(RenderDevice::COLORWRITEENABLE, 0);         // write only to depth buffer
         pd3dDevice->basicShader->Begin(0);
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX, vertices+16, 16, rgi, 3*14);
         pd3dDevice->basicShader->End();
         pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
         pd3dDevice->SetRenderState(RenderDevice::COLORWRITEENABLE, 0x0f);      // re-enable color writes
         break;
      }

      case KickerCup: 
      {
         WORD rgi[2*3*16];

         // Draw cup
         for (int l=0;l<16;++l)
         {
            rgi[l*3 ] = 32;
            rgi[l*3+1] = l;
            rgi[l*3+2] = (l + 1)%16;

            SetNormal(vertices+16, rgi+l*3, 3, NULL, NULL, 0);

            vertices[48].nx = 0;
            vertices[48].ny = 0;
            vertices[48].nz = 1.0f;
         }
         pd3dDevice->basicShader->Begin(0);
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX, vertices+16, 49-16, rgi, 3*16);
         
         // Draw outer ring
         for (int l=0; l<16; ++l)
         {
            rgi[l*6  ] = l;
            rgi[l*6+1] = 16 + l;
            rgi[l*6+2] = 16 + (l + 1)%16;

            rgi[l*6+3] = l;
            rgi[l*6+4] = 16 + (l + 1)%16;
            rgi[l*6+5] = (l + 1)%16;

            SetNormal(vertices+16, rgi+l*6, 6, NULL, NULL, 0);
         }
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX, vertices+16, 32, rgi, 2*3*16);
         pd3dDevice->basicShader->End();
         // Draw the top "lid" of the kicker hole invisibly (for the depth buffer)
         for (int l=0; l<14; ++l)
         {
            rgi[l*3  ] = 0;
            rgi[l*3+1] = l+1;
            rgi[l*3+2] = l+2;

            SetNormal(vertices+32, rgi+l*3, 3, NULL, NULL, 0);
         }
         float depthbias = -1e-4f;
         pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, *((DWORD*)&depthbias));
         pd3dDevice->SetRenderState(RenderDevice::COLORWRITEENABLE, 0);         // write only to depth buffer
         pd3dDevice->basicShader->Begin(0);
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX, vertices+32, 16, rgi, 3*14);
         pd3dDevice->basicShader->End();
         pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
         pd3dDevice->SetRenderState(RenderDevice::COLORWRITEENABLE, 0x0f);      // re-enable color writes
         break;
      }
   }

   ppin3d->DisableLightMap();
}

void Kicker::RenderStatic(RenderDevice* pd3dDevice)
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

	bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
	bw.WriteFloat(FID(RADI), m_d.m_radius);
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteString(FID(MATR), m_d.m_szMaterial);
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

	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
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
	else if (id == FID(MATR))
		{
		pbr->GetString(m_d.m_szMaterial);
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

	for (size_t i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
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

		pball->m_coll.normal[1].x = 1.0f;           // HACK: avoid capture leaving kicker
		m_phitkickercircle->DoCollide(pball, NULL); //
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

		pball->m_coll.normal[1].x = 1.0f;           // HACK: avoid capture leaving kicker
		m_phitkickercircle->DoCollide(pball, NULL); //
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
		
        //!! TODO: reenable scatter if desired
		//float scatterAngle = (m_d.m_scatter <= 0.0f) ? c_hardScatter : ANGTORAD(m_d.m_scatter); // if <= 0 use global value
		//scatterAngle *= g_pplayer->m_ptable->m_globalDifficulty;		// apply dificulty weighting

		//if (scatterAngle > 1.0e-5f)										// ignore near zero angles
		//	{
		//	float scatter = rand_mt_m11();								// -1.0f..1.0f
		//	scatter *= (1.0f - scatter*scatter)*2.59808f * scatterAngle;// shape quadratic distribution and scale
		//	anglerad += scatter;
		//	}
		
		const float speedz = sinf(inclination) * speed;

		if (speedz > 0.f)
			speed = cosf(inclination) * speed;

		m_phitkickercircle->m_pball->pos.x += x; // brian's suggestion
		m_phitkickercircle->m_pball->pos.y += y; 
		m_phitkickercircle->m_pball->pos.z += z; 

		m_phitkickercircle->m_pball->vel.x =  sinf(anglerad) * speed;
		m_phitkickercircle->m_pball->vel.y = -cosf(anglerad) * speed;
		m_phitkickercircle->m_pball->vel.z = speedz;
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

STDMETHODIMP Kicker::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Kicker::put_Material(BSTR newVal)
{
	STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Kicker::BallCntOver(int *pVal)
{
	int cnt = 0;

	if (g_pplayer)
		{
		for (unsigned i = 0; i < g_pplayer->m_vball.size(); i++)
			{
			Ball * const pball = g_pplayer->m_vball[i];

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

float KickerHitCircle::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{
	return HitTestBasicRadius(pball, dtime, coll, false, false, false); //any face, not-lateral, non-rigid
}

void KickerHitCircle::DoCollide(Ball * const pball, Vertex3Ds * const phitnormal)
	{
	if (m_pball) return;								// a previous ball already in kicker

	const int i = pball->m_vpVolObjs->IndexOf(m_pObj);	// check if kicker in ball's volume set

	if (!phitnormal || ((phitnormal[1].x < 1) == (i < 0))) // New or (Hit && !Vol || UnHit && Vol)
		{
        pball->pos += STATICTIME * pball->vel;          // move ball slightly forward

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
				pball->vel.SetZero();
				pball->pos.x = center.x;
				pball->pos.y = center.y;
				pball->pos.z = m_zheight + pball->radius;
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
