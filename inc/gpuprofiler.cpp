#include "stdafx.h"
#include "gpuprofiler.h"
#include <d3d9.h>

#define FLUSH_DATA /*0*/ D3DGETDATA_FLUSH // latter should be a bit less accurate but leading to less failures when getting the data
#define GET_DATA_RETRIES 10

#define ErrorPrintf(x, ...) { char sz[256]; sprintf_s(sz,x,__VA_ARGS__); ShowError(sz); }
#ifdef _DEBUG
 #define DebugPrintf(x, ...) { char sz[256]; sprintf_s(sz,x,__VA_ARGS__); ShowError(sz); }
#else
 #define DebugPrintf(x, ...)
#endif

CGpuProfiler::CGpuProfiler ()
:   m_init(false),
	m_iFrameQuery(0),
	m_iFrameCollect(-1),
	m_frameCountAvg(0),
	m_tBeginAvg(0.0),
	m_frequencyQuery(NULL),
	m_device(NULL)
{
	memset(m_apQueryTsDisjoint, 0, sizeof(m_apQueryTsDisjoint));
	memset(m_apQueryTs, 0, sizeof(m_apQueryTs));
	memset(m_adT, 0, sizeof(m_adT));
	memset(m_adTAvg, 0, sizeof(m_adTAvg));
	memset(m_adTTotalAvg, 0, sizeof(m_adTTotalAvg));
}

CGpuProfiler::~CGpuProfiler()
{
	Shutdown();
}

bool CGpuProfiler::Init (IDirect3DDevice9 * const pDevice)
{
	m_init = true;

	const HRESULT tsHr  = pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMP, NULL);
	const HRESULT tsdHr = pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMPDISJOINT, NULL);
	const HRESULT tsfHr = pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMPFREQ, NULL);

	if (tsHr || tsdHr || tsfHr)
	{
		ErrorPrintf("GPU Profiler: Query not supported");
		return false;
	}

	// Create all the queries we'll need
	if (FAILED(pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMPDISJOINT, &m_apQueryTsDisjoint[0])))
	{
		ErrorPrintf("GPU Profiler: Could not create timestamp disjoint query for frame 0!");
		return false;
	}

	if (FAILED(pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMPDISJOINT, &m_apQueryTsDisjoint[1])))
	{
		ErrorPrintf("GPU Profiler: Could not create timestamp disjoint query for frame 1!");
		return false;
	}

	for (GTS gts = GTS_BeginFrame; gts < GTS_Max; gts = GTS(gts + 1))
	{
		if (FAILED(pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &m_apQueryTs[gts][0])))
		{
			ErrorPrintf("GPU Profiler: Could not create start-frame timestamp query for GTS %d, frame 0!", gts);
			return false;
		}

		if (FAILED(pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &m_apQueryTs[gts][1])))
		{
			ErrorPrintf("GPU Profiler: Could not create start-frame timestamp query for GTS %d, frame 1!", gts);
			return false;
		}
	}

	if(FAILED(pDevice->CreateQuery(D3DQUERYTYPE_TIMESTAMPFREQ, &m_frequencyQuery)))
	{
		ErrorPrintf("GPU Profiler: Could not create frequency query!");
		return false;
	}

	m_device = pDevice;

	return true;
}

void CGpuProfiler::ResetCounters()
{
	m_frameCountAvg = 0;
	m_tBeginAvg = 0.0;

	memset(m_adT, 0, sizeof(m_adT));
	memset(m_adTAvg, 0, sizeof(m_adTAvg));
	memset(m_adTTotalAvg, 0, sizeof(m_adTTotalAvg));
}

void CGpuProfiler::Shutdown ()
{
	if (m_apQueryTsDisjoint[0])
	{
		m_apQueryTsDisjoint[0]->Release();
		m_apQueryTsDisjoint[0] = 0;
	}

	if (m_apQueryTsDisjoint[1])
	{
		m_apQueryTsDisjoint[1]->Release();
		m_apQueryTsDisjoint[1] = 0;
	}

	for (GTS gts = GTS_BeginFrame; gts < GTS_Max; gts = GTS(gts + 1))
	{
		if (m_apQueryTs[gts][0])
		{
			m_apQueryTs[gts][0]->Release();
			m_apQueryTs[gts][0] = 0;
		}

		if (m_apQueryTs[gts][1])
		{
			m_apQueryTs[gts][1]->Release();
			m_apQueryTs[gts][1] = 0;
		}
	}

	if (m_frequencyQuery)
	{
		m_frequencyQuery->Release();
		m_frequencyQuery = 0;
	}

	m_init = false;
	m_device = NULL;

	m_iFrameQuery = 0;
	m_iFrameCollect = -1;

	ResetCounters();
}

void CGpuProfiler::BeginFrame(IDirect3DDevice9 * const pDevice)
{
	if (!m_init)
		Init(pDevice);

	if (!m_device)
		return;

	for (GTS gts = GTS_BeginFrame; gts < GTS_Max; gts = GTS(gts + 1))
		m_apQueryTs_triggered[gts][m_iFrameQuery] = false;

	m_frequencyQuery->Issue(D3DISSUE_END);
	m_apQueryTsDisjoint[m_iFrameQuery]->Issue(D3DISSUE_END);
	Timestamp(GTS_BeginFrame);
}

void CGpuProfiler::Timestamp (GTS gts)
{
	if (!m_device)
		return;

	m_apQueryTs_triggered[gts][m_iFrameQuery] = true;
	m_apQueryTs[gts][m_iFrameQuery]->Issue(D3DISSUE_END);
}

void CGpuProfiler::EndFrame ()
{
	if (!m_device)
		return;

	Timestamp(GTS_EndFrame);
	m_apQueryTsDisjoint[m_iFrameQuery]->Issue(D3DISSUE_END);

	m_iFrameQuery = (m_iFrameQuery+1)&1;
}

void CGpuProfiler::WaitForDataAndUpdate ()
{
	if (!m_device)
		return;

	if (m_iFrameCollect < 0)
	{
		// Haven't run enough frames yet to have data
		m_iFrameCollect = 0;
		return;
	}

	// Wait for data
	UINT32 c = 0;
	BOOL disjoint;
	while (m_apQueryTsDisjoint[m_iFrameCollect]->GetData(&disjoint,sizeof(BOOL), FLUSH_DATA) == S_FALSE && c < GET_DATA_RETRIES)
	{
		c++;
		Sleep(1); //!!
	}
	if(c >= GET_DATA_RETRIES)
	{
		DebugPrintf("GPU Profiler: Failed while waiting for data");
		return;
	}

	if(disjoint)
	{
		DebugPrintf("GPU Profiler: Timing interval disjoint");
		return;
	}

	const int iFrame = m_iFrameCollect;
	m_iFrameCollect = (m_iFrameCollect+1)&1;

	UINT64 frequency;
	c = 0;
	while (m_frequencyQuery->GetData(&frequency, sizeof(UINT64), FLUSH_DATA) == S_FALSE && c < GET_DATA_RETRIES)
	{
		c++;
		Sleep(1); //!!
	}
	if (c >= GET_DATA_RETRIES)
	{
		DebugPrintf("GPU Profiler: Couldn't retrieve frequency data");
		return;
	}

	UINT64 timestampPrev;
	c = 0;
	while (m_apQueryTs[GTS_BeginFrame][iFrame]->GetData(&timestampPrev, sizeof(UINT64), FLUSH_DATA) == S_FALSE && c < GET_DATA_RETRIES)
	{
		c++;
		Sleep(1); //!!
	}
	if (c >= GET_DATA_RETRIES)
	{
		//DebugPrintf("GPU Profiler: Couldn't retrieve timestamp query data for GTS %d", GTS_BeginFrame); //!! disabled for now as it still gets triggered if rendering with 4xAA on my machine

		ResetCounters();

		return;
	}

	for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_Max; gts = GTS(gts + 1))
	{
		UINT64 timestamp = timestampPrev;
		if (m_apQueryTs_triggered[gts][iFrame])
		{
			c = 0;
			while (m_apQueryTs[gts][iFrame]->GetData(&timestamp, sizeof(UINT64), FLUSH_DATA) == S_FALSE && c < GET_DATA_RETRIES)
			{
				c++;
				Sleep(1); //!!
			}
			if (c >= GET_DATA_RETRIES)
			{
				//DebugPrintf("GPU Profiler: Couldn't retrieve timestamp query data for GTS %d", gts); //!! disabled for now as it still gets triggered if rendering is extremely fast/high FPS
				//return;

				timestamp = timestampPrev;
			}
		}

		m_adT[gts] = (double)(timestamp - timestampPrev) / (double)frequency;
		timestampPrev = timestamp;

		m_adTTotalAvg[gts] += m_adT[gts];
	}

	++m_frameCountAvg;
	const unsigned int ms = msec();
	const double t = (double)ms*1e-3;
	if (t > m_tBeginAvg + 0.5)
	{
		for (GTS gts = GTS_BeginFrame; gts < GTS_Max; gts = GTS(gts + 1))
		{
			m_adTAvg[gts] = m_adTTotalAvg[gts] / (double)m_frameCountAvg;
			m_adTTotalAvg[gts] = 0.0;
		}

		m_frameCountAvg = 0;
		m_tBeginAvg = t;
	}
}
