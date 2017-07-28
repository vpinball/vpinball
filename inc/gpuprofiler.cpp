#include "gpuprofiler.hpp"
#include <d3d11.h>

extern ID3D11Device * g_pDevice;
extern ID3D11DeviceContext * g_pContext;
void ErrorPrintf (const char * message, ...);
void DebugPrintf (const char * message, ...);
float Time ();		// Retrieve time in seconds, using QueryPerformanceCounter or whatever


// CGpuProfiler implementation

CGpuProfiler g_gpuProfiler;

CGpuProfiler::CGpuProfiler ()
:	m_iFrameQuery(0),
	m_iFrameCollect(-1),
	m_frameCountAvg(0),
	m_tBeginAvg(0.0f)
{
	memset(m_apQueryTsDisjoint, 0, sizeof(m_apQueryTsDisjoint));
	memset(m_apQueryTs, 0, sizeof(m_apQueryTs));
	memset(m_adT, 0, sizeof(m_adT));
	memset(m_adTAvg, 0, sizeof(m_adT));
	memset(m_adTTotalAvg, 0, sizeof(m_adT));
}

bool CGpuProfiler::Init ()
{
	// Create all the queries we'll need

	D3D11_QUERY_DESC queryDesc = { D3D11_QUERY_TIMESTAMP_DISJOINT, 0 };

	if (FAILED(g_pDevice->CreateQuery(&queryDesc, &m_apQueryTsDisjoint[0])))
	{
		ErrorPrintf("Could not create timestamp disjoint query for frame 0!");
		return false;
	}

	if (FAILED(g_pDevice->CreateQuery(&queryDesc, &m_apQueryTsDisjoint[1])))
	{
		ErrorPrintf("Could not create timestamp disjoint query for frame 1!");
		return false;
	}

	queryDesc.Query = D3D11_QUERY_TIMESTAMP;

	for (GTS gts = GTS_BeginFrame; gts < GTS_Max; gts = GTS(gts + 1))
	{
		if (FAILED(g_pDevice->CreateQuery(&queryDesc, &m_apQueryTs[gts][0])))
		{
			ErrorPrintf("Could not create start-frame timestamp query for GTS %d, frame 0!", gts);
			return false;
		}

		if (FAILED(g_pDevice->CreateQuery(&queryDesc, &m_apQueryTs[gts][1])))
		{
			ErrorPrintf("Could not create start-frame timestamp query for GTS %d, frame 1!", gts);
			return false;
		}
	}

	return true;
}

void CGpuProfiler::Shutdown ()
{
	if (m_apQueryTsDisjoint[0])
		m_apQueryTsDisjoint[0]->Release();

	if (m_apQueryTsDisjoint[1])
		m_apQueryTsDisjoint[1]->Release();

	for (GTS gts = GTS_BeginFrame; gts < GTS_Max; gts = GTS(gts + 1))
	{
		if (m_apQueryTs[gts][0])
			m_apQueryTs[gts][0]->Release();

		if (m_apQueryTs[gts][1])
			m_apQueryTs[gts][1]->Release();
	}
}

void CGpuProfiler::BeginFrame ()
{
	g_pContext->Begin(m_apQueryTsDisjoint[m_iFrameQuery]);
	Timestamp(GTS_BeginFrame);
}

void CGpuProfiler::Timestamp (GTS gts)
{
	g_pContext->End(m_apQueryTs[gts][m_iFrameQuery]);
}

void CGpuProfiler::EndFrame ()
{
	Timestamp(GTS_EndFrame);
	g_pContext->End(m_apQueryTsDisjoint[m_iFrameQuery]);

	++m_iFrameQuery &= 1;
}

void CGpuProfiler::WaitForDataAndUpdate ()
{
	if (m_iFrameCollect < 0)
	{
		// Haven't run enough frames yet to have data
		m_iFrameCollect = 0;
		return;
	}

	// Wait for data
	while (g_pContext->GetData(m_apQueryTsDisjoint[m_iFrameCollect], NULL, 0, 0) == S_FALSE)
	{
		Sleep(1);
	}

	int iFrame = m_iFrameCollect;
	++m_iFrameCollect &= 1;

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT timestampDisjoint;
	if (g_pContext->GetData(m_apQueryTsDisjoint[iFrame], &timestampDisjoint, sizeof(timestampDisjoint), 0) != S_OK)
	{
		DebugPrintf("Couldn't retrieve timestamp disjoint query data");
		return;
	}

	if (timestampDisjoint.Disjoint)
	{
		// Throw out this frame's data
		DebugPrintf("Timestamps disjoint");
		return;
	}

	UINT64 timestampPrev;
	if (g_pContext->GetData(m_apQueryTs[GTS_BeginFrame][iFrame], &timestampPrev, sizeof(UINT64), 0) != S_OK)
	{
		DebugPrintf("Couldn't retrieve timestamp query data for GTS %d", GTS_BeginFrame);
		return;
	}

	for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_Max; gts = GTS(gts + 1))
	{
		UINT64 timestamp;
		if (g_pContext->GetData(m_apQueryTs[gts][iFrame], &timestamp, sizeof(UINT64), 0) != S_OK)
		{
			DebugPrintf("Couldn't retrieve timestamp query data for GTS %d", gts);
			return;
		}

		m_adT[gts] = float(timestamp - timestampPrev) / float(timestampDisjoint.Frequency);
		timestampPrev = timestamp;

		m_adTTotalAvg[gts] += m_adT[gts];
	}

	++m_frameCountAvg;
	if (Time() > m_tBeginAvg + 0.5f)
	{
		for (GTS gts = GTS_BeginFrame; gts < GTS_Max; gts = GTS(gts + 1))
		{
			m_adTAvg[gts] = m_adTTotalAvg[gts] / m_frameCountAvg;
			m_adTTotalAvg[gts] = 0.0f;
		}

		m_frameCountAvg = 0;
		m_tBeginAvg = Time();
	}
}
