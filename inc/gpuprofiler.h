// GPU performance-measurement subsystem

#pragma once

struct ID3D11DeviceContext;
struct ID3D11Query;


// Enum of GPU timestamps to record
enum GTS
{
	GTS_BeginFrame,
	GTS_ShadowClear,
	GTS_ShadowObjects,
	GTS_MainClear,
	GTS_MainObjects,
	GTS_EndFrame,

	GTS_Max
};

class CGpuProfiler
{
public:
	CGpuProfiler ();

	bool Init ();
	void Shutdown ();

	void BeginFrame ();
	void Timestamp (GTS gts);
	void EndFrame ();

	// Wait on GPU for last frame's data (not this frame's) to be available
	void WaitForDataAndUpdate ();

	float Dt (GTS gts)
		{ return m_adT[gts]; }
	float DtAvg (GTS gts)
		{ return m_adTAvg[gts]; }

protected:
	int m_iFrameQuery;							// Which of the two sets of queries are we currently issuing?
	int m_iFrameCollect;						// Which of the two did we last collect?
	ID3D11Query * m_apQueryTsDisjoint[2];		// "Timestamp disjoint" query; records whether timestamps are valid
	ID3D11Query * m_apQueryTs[GTS_Max][2];		// Individual timestamp queries for each relevant point in the frame

	float m_adT[GTS_Max];						// Last frame's timings (each relative to previous GTS)
	float m_adTAvg[GTS_Max];					// Timings averaged over 0.5 second

	float m_adTTotalAvg[GTS_Max];				// Total timings thus far within this averaging period
	int m_frameCountAvg;						// Frames rendered in current averaging period
	float m_tBeginAvg;							// Time at which current averaging period started
};

extern CGpuProfiler g_gpuProfiler;
