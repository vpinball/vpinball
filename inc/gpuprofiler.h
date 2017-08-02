// GPU performance-measurement subsystem

// Original license for the original D3D11 codebase coming up, code is modified though by VP team to work with D3D9 (and more)

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
	GTS_BeginFrame = 0,
	GTS_PlayfieldGraphics = 1,
	GTS_Primitives_NT = 1, // eItemPrimitive
	GTS_NonTransparent = 2,
	GTS_Walls_Ramps_Rubbers_NT = 2, // eItemSurface eItemRamp eItemRubber
	GTS_LightBuffer = 3,
	GTS_Else = 3, // eItemTextbox eItemDispReel	eItemDecal eItemFlipper	eItemPlunger eItemBumper eItemHitTarget	eItemTrigger eItemKicker eItemGate eItemSpinner
	GTS_Transparent = 4,
	GTS_Walls_Ramps_Rubbers_T = 4, // eItemSurface eItemRamp eItemRubber
	GTS_Bloom = 5,
	GTS_Primitives_T = 5, // eItemPrimitive
	GTS_AO = 6,
	GTS_Lights = 6, // eItemLight
	GTS_PostProcess = 7,
	GTS_Flashers = 7, //eItemFlasher
	GTS_EndFrame = 8,

	GTS_Max = 9
};

static const char * GTS_name_item[GTS_Max] =
{
	"Begin Frame",
	"Primitives (Non-Transparent)",
	"Walls/Ramps/Rubbers (Non-Transparent)",
	"Everything else",
	"Walls/Ramps/Rubbers (Transparent)",
	"Primitives (Transparent)",
	"Lights",
	"Flashers",
	"End Frame"
};

static const char * GTS_name[GTS_Max] =
{
	"Begin Frame",
	"Playfield Graphics/Reflections",
	"Non-Transparent Elements",
	"Light Buffer",
	"Transparent Elements",
	"Bloom",
	"Ambient Occlusion",
	"Post Processing",
	"End Frame"
};

class CGpuProfiler
{
public:
	CGpuProfiler();
	~CGpuProfiler();

	bool Init(IDirect3DDevice9 * const pDevice); // optional init, otherwise (first) BeginFrame will do it lazily
	void Shutdown();

	void BeginFrame(IDirect3DDevice9 * const pDevice);
	void Timestamp(const GTS gts);
	void EndFrame();

	// Wait on GPU for last frame's data (not this frame's) to be available
	void WaitForDataAndUpdate();

	void ResetCounters();

	double Dt(const GTS gts)    { return m_adT[gts]; }
	double DtAvg(const GTS gts) { return m_adTAvg[gts]; }

protected:
	bool m_init;

	int m_iFrameQuery;							// Which of the two sets of queries are we currently issuing?
	int m_iFrameCollect;						// Which of the two did we last collect?
	IDirect3DQuery9 * m_apQueryTsDisjoint[2];	// "Timestamp disjoint" query; records whether timestamps are valid
	IDirect3DQuery9 * m_apQueryTs[GTS_Max][2];	// Individual timestamp queries for each relevant point in the frame
	IDirect3DQuery9 * m_frequencyQuery;

	bool m_apQueryTs_triggered[GTS_Max][2];     // Check which queries actually were triggered in the frame

	double m_adT[GTS_Max];						// Last frame's timings (each relative to previous GTS)
	double m_adTAvg[GTS_Max];					// Timings averaged over 0.5 second

	double m_adTTotalAvg[GTS_Max];				// Total timings thus far within this averaging period
	int m_frameCountAvg;						// Frames rendered in current averaging period
	double m_tBeginAvg;							// Time at which current averaging period started

	IDirect3DDevice9 * m_device;
};
