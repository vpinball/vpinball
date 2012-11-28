// Timer.cpp : Implementation of CVBATestApp and DLL registration.

#include "StdAfx.h"
#include "VBATest.h"

/////////////////////////////////////////////////////////////////////////////
//

Timer::Timer()
	{
	}

Timer::~Timer()
	{
	}

HRESULT Timer::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
	{
	m_ptable = ptable;

	m_d.m_v.x = x;
	m_d.m_v.y = y;

	SetDefaults(fromMouseClick);

	return InitVBA(fTrue, 0, NULL);//ApcProjectItem.Define(ptable->ApcProject, GetDispatch(),
		//axTypeHostProjectItem/*axTypeHostClass*/, L"Timer", NULL);
	}

void Timer::SetDefaults(bool fromMouseClick)
	{
	HRESULT hr;
	int iTmp;

	hr = GetRegInt("DefaultProps\\Timer","TimerEnabled", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_tdr.m_fTimerEnabled = iTmp == 0? false:true;
	else
		m_d.m_tdr.m_fTimerEnabled = true;
	
	hr = GetRegInt("DefaultProps\\Timer","TimerInterval", &iTmp);
	if ((hr == S_OK) && fromMouseClick)
		m_d.m_tdr.m_TimerInterval = iTmp;
	else
		m_d.m_tdr.m_TimerInterval = 100;
	}

void Timer::WriteRegDefaults()
	{
	SetRegValue("DefaultProps\\Timer","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
	SetRegValue("DefaultProps\\Timer","TimerInterval",REG_DWORD,&m_d.m_tdr.m_TimerInterval,4);
	}

void Timer::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_v.x, m_d.m_v.y);
	}

void Timer::MoveOffset(const float dx, const float dy)
	{
	m_d.m_v.x += dx;
	m_d.m_v.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Timer::GetCenter(Vertex2D * const pv) const
	{
	*pv = m_d.m_v;
	}

void Timer::PutCenter(const Vertex2D * const pv)
	{
	m_d.m_v = *pv;

	m_ptable->SetDirtyDraw();
	}

void Timer::PreRender(Sur *psur)
	{
	}

void Timer::Render(Sur *psur)
	{
	psur->SetFillColor(-1);//RGB(192,192,192));
	psur->SetBorderColor(RGB(0,0,0),false,0);
	psur->SetLineColor(RGB(0,0,0), false, 0);

	psur->SetObject(this);

	psur->Ellipse(m_d.m_v.x, m_d.m_v.y, 18);

	psur->Ellipse(m_d.m_v.x, m_d.m_v.y, 15);

	for (int i=0;i<12;i++)
		{
		const float angle = (float)(M_PI*2.0/12.0)*(float)i;
		const float sn = sinf(angle);
		const float cs = cosf(angle);
		psur->Line(m_d.m_v.x + sn*9.0f, m_d.m_v.y - cs*9.0f, m_d.m_v.x + sn*15, m_d.m_v.y - cs*15.0f);
		}

	//angle = ((PI*2)/24) * 3;
	psur->Line(m_d.m_v.x, m_d.m_v.y, m_d.m_v.x + 10.5f, m_d.m_v.y - 7.5f);

	}

void Timer::RenderBlueprint(Sur *psur)
	{
	}

void Timer::GetTimers(Vector<HitTimer> *pvht)
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

void Timer::GetHitShapes(Vector<HitObject> *pvho)
	{
	m_phittimer = NULL;
	}

void Timer::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}

void Timer::EndPlay()
	{
	IEditable::EndPlay();
	}

void Timer::PostRenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

void Timer::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}
	
void Timer::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

STDMETHODIMP Timer::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] =
	{
		&IID_ITimer,
	};

	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP Timer::get_Enabled(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_tdr.m_fTimerEnabled);

	return S_OK;
}

STDMETHODIMP Timer::put_Enabled(VARIANT_BOOL newVal)
{
	STARTUNDO

	const BOOL fNew = VBTOF(newVal);

	if (fNew != m_d.m_tdr.m_fTimerEnabled)
		{
		if (m_phittimer)
			{
			if (fNew)
				{
				m_phittimer->m_nextfire = g_pplayer->m_timeCur + m_phittimer->m_interval;
				g_pplayer->m_vht.AddElement(m_phittimer);
				}
			else
				{
				g_pplayer->m_vht.RemoveElement(m_phittimer);
				}
			}
		}

	m_d.m_tdr.m_fTimerEnabled = fNew;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Timer::get_Interval(long *pVal)
{
	*pVal = m_d.m_tdr.m_TimerInterval;

	return S_OK;
}

STDMETHODIMP Timer::put_Interval(long newVal)
{
	STARTUNDO

	m_d.m_tdr.m_TimerInterval = newVal;

	if (m_phittimer)
		{
		m_phittimer->m_interval = m_d.m_tdr.m_TimerInterval;
		m_phittimer->m_nextfire = g_pplayer->m_timeCur + m_phittimer->m_interval;
		}

	STOPUNDO

	return S_OK;
}

HRESULT Timer::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
	bw.WriteStruct(FID(VCEN), &m_d.m_v, sizeof(Vertex2D));
	bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
	bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

	bw.WriteBool(FID(BGLS), m_fBackglass);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	}

HRESULT Timer::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults(false);
#ifndef OLDLOAD
	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
#else
	m_ptable = ptable;

	ULONG read = 0;
	DWORD dwID;
	HRESULT hr;
	if(FAILED(hr = pstm->Read(&dwID, sizeof dwID, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_d, sizeof(TimerData), &read)))
		return hr;

	//ApcProjectItem.Register(ptable->ApcProject, GetDispatch(), dwID);
	*pid = dwID;

	return hr;
#endif
	}

BOOL Timer::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VCEN))
		{
		pbr->GetStruct(&m_d.m_v, sizeof(Vertex2D));
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
	else if (id == FID(BGLS))
		{
		pbr->GetBool(&m_fBackglass);
		}
	else
		{
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT Timer::InitPostLoad()
	{
	return S_OK;
	}

void Timer::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
	pvproppane->AddElement(pproppane);
	}
