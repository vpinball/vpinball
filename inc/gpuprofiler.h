// GPU performance-measurement subsystem

// Original license for the D3D11 codebase coming up, code is modified though by VP team to work with D3D9 (and more)

/*
Copyright(c) 2011 by Nathan Reed.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and / or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.IN NO
EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

// Enum of GPU timestamps to record
enum GTS
{
	GTS_BeginFrame,
	GTS_PlayfieldGraphics,
	GTS_NonTransparent,
	GTS_LightBuffer,
	GTS_Transparent,
	GTS_AO,
	GTS_PostProcess,
	GTS_EndFrame,

	GTS_Max
};

class CGpuProfiler
{
public:
	CGpuProfiler ();

	bool Init (IDirect3DDevice9 * const pDevice);
	void Shutdown ();

	void BeginFrame ();
	void Timestamp (const GTS gts);
	void EndFrame ();

	// Wait on GPU for last frame's data (not this frame's) to be available
	void WaitForDataAndUpdate ();

	double Dt (const GTS gts)    { return m_adT[gts]; }
	double DtAvg(const GTS gts)  { return m_adTAvg[gts]; }

protected:
	int m_iFrameQuery;							// Which of the two sets of queries are we currently issuing?
	int m_iFrameCollect;						// Which of the two did we last collect?
	IDirect3DQuery9 * m_apQueryTsDisjoint[2];	// "Timestamp disjoint" query; records whether timestamps are valid
	IDirect3DQuery9 * m_apQueryTs[GTS_Max][2];	// Individual timestamp queries for each relevant point in the frame
	IDirect3DQuery9 * m_frequencyQuery;

	double m_adT[GTS_Max];						// Last frame's timings (each relative to previous GTS)
	double m_adTAvg[GTS_Max];					// Timings averaged over 0.5 second

	double m_adTTotalAvg[GTS_Max];				// Total timings thus far within this averaging period
	int m_frameCountAvg;						// Frames rendered in current averaging period
	double m_tBeginAvg;							// Time at which current averaging period started

	IDirect3DDevice9 * m_device;
};
