/*		FAAA - Antialiasing shader by G.Rebord
		based on FXAA 3.11 by TIMOTHY LOTTES */

// Edge detection threshold. Higher values result in more edges being
// detected and smoothed. Range: 1.0 to 9.0. Default: 5.0 (thorough)
#define FAAA_EDGE_THRESHOLD 9.0 // was 5 in original FAAA
// Antialiasing quality setting. Higher values result in higher quality
// of antialiasing applied to detected edges. Default: 5 (high quality)
// Range: From 1 (fastest) to 9 (highest quality).
#define FAAA_QUALITY 9 // was 5 in original FAAA

/*============================================================================
	Settings - from FXAA3 QUALITY VERSION LOW DITHER SETTINGS
============================================================================*/
#if (FAAA_QUALITY == 1)
	#define FAAA_QUALITY__PI  4
	#define FAAA_QUALITY__P3  8.0
#endif
#if (FAAA_QUALITY == 2)
	#define FAAA_QUALITY__PI  5
	#define FAAA_QUALITY__P4  8.0
#endif
#if (FAAA_QUALITY == 3)
	#define FAAA_QUALITY__PI  6
	#define FAAA_QUALITY__P5  8.0
#endif
#if (FAAA_QUALITY == 4)
	#define FAAA_QUALITY__PI  7
	#define FAAA_QUALITY__P5  3.0
	#define FAAA_QUALITY__P6  8.0
#endif
#if (FAAA_QUALITY == 5)
	#define FAAA_QUALITY__PI  8
	#define FAAA_QUALITY__P6  4.0
	#define FAAA_QUALITY__P7  8.0
#endif
#if (FAAA_QUALITY == 6)
	#define FAAA_QUALITY__PI  9
	#define FAAA_QUALITY__P7  4.0
	#define FAAA_QUALITY__P8  8.0
#endif
#if (FAAA_QUALITY == 7)
	#define FAAA_QUALITY__PI  10
	#define FAAA_QUALITY__P8  4.0
	#define FAAA_QUALITY__P9  8.0
#endif
#if (FAAA_QUALITY == 8)
	#define FAAA_QUALITY__PI  11
	#define FAAA_QUALITY__P9  4.0
	#define FAAA_QUALITY__P10 8.0
#endif
#if (FAAA_QUALITY == 9)
	#define FAAA_QUALITY__PI  12
	#define FAAA_QUALITY__P1  1.0
	#define FAAA_QUALITY__P2  1.0
	#define FAAA_QUALITY__P3  1.0
	#define FAAA_QUALITY__P4  1.0
	#define FAAA_QUALITY__P5  1.5
#endif
/*--------------------------------------------------------------------------*/
#define FAAA_QUALITY__P0  1.0
#define FAAA_QUALITY__P11 8.0
/*--------------------------------------------------------------------------*/
#if !defined FAAA_QUALITY__P1
	#define  FAAA_QUALITY__P1  1.5
#endif
#if !defined FAAA_QUALITY__P2
	#define  FAAA_QUALITY__P2  2.0
#endif
#if !defined FAAA_QUALITY__P3
	#define  FAAA_QUALITY__P3  2.0
#endif
#if !defined FAAA_QUALITY__P4
	#define  FAAA_QUALITY__P4  2.0
#endif
#if !defined FAAA_QUALITY__P5
	#define  FAAA_QUALITY__P5  2.0
#endif
#if !defined FAAA_QUALITY__P6
	#define  FAAA_QUALITY__P6  2.0
#endif
#if !defined FAAA_QUALITY__P7
	#define  FAAA_QUALITY__P7  2.0
#endif
#if !defined FAAA_QUALITY__P8
	#define  FAAA_QUALITY__P8  2.0
#endif
#if !defined FAAA_QUALITY__P9
	#define  FAAA_QUALITY__P9  2.0
#endif
#if !defined FAAA_QUALITY__P10
	#define  FAAA_QUALITY__P10  4.0
#endif

float3 faaa(const float2 u)
{
#ifdef GLSL
	const float FAAAOffMult[12] = float[]( FAAA_QUALITY__P0, FAAA_QUALITY__P1,
		FAAA_QUALITY__P2, FAAA_QUALITY__P3, FAAA_QUALITY__P4, FAAA_QUALITY__P5,
		FAAA_QUALITY__P6, FAAA_QUALITY__P7, FAAA_QUALITY__P8, FAAA_QUALITY__P9,
		FAAA_QUALITY__P10, FAAA_QUALITY__P11 );
#else
	const float FAAAOffMult[12] = { FAAA_QUALITY__P0, FAAA_QUALITY__P1,
		FAAA_QUALITY__P2, FAAA_QUALITY__P3, FAAA_QUALITY__P4, FAAA_QUALITY__P5,
		FAAA_QUALITY__P6, FAAA_QUALITY__P7, FAAA_QUALITY__P8, FAAA_QUALITY__P9,
		FAAA_QUALITY__P10, FAAA_QUALITY__P11 };
#endif

	const float3 rgbyM = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
			return rgbyM;
	}

	const float2 offs = w_h_height.xy;
	const float lumaSE = luma(texStereoNoLod(tex_fb_unfiltered, u + offs).xyz);
	const float lumaNW = luma(texStereoNoLod(tex_fb_unfiltered, u - offs).xyz);
	const float lumaSW = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(-offs.x,offs.y)).xyz);
	const float lumaNE = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(-offs.x,offs.y)).xyz);
	// Edge detection. This does much better than a head-on luma delta.
	const float gradientSWNE = lumaSW - lumaNE;
	const float gradientSENW = lumaSE - lumaNW;
	const float2 dirM = float2(abs(gradientSWNE + gradientSENW), abs(gradientSWNE - gradientSENW));
	const float lumaMax = max(max(lumaSW, lumaSE), max(lumaNE, lumaNW));
	const float localLumaFactor = lumaMax * 0.5 + 0.5;
	const float localThres = ((10.0 - FAAA_EDGE_THRESHOLD) * 0.0625) * localLumaFactor;
	const bool lowDelta = abs(dirM.x - dirM.y) < localThres;
	BRANCH if(lowDelta)
		return rgbyM;

	const bool horzSpan = dirM.x > dirM.y;
	const float lumaM = luma(rgbyM);
	float lumaN, lumaS;
	if( horzSpan) lumaN = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(0.0,offs.y)).xyz);
	if( horzSpan) lumaS = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(0.0,offs.y)).xyz);
	if(!horzSpan) lumaN = luma(texStereoNoLod(tex_fb_unfiltered, u - float2(offs.x,0.0)).xyz);
	if(!horzSpan) lumaS = luma(texStereoNoLod(tex_fb_unfiltered, u + float2(offs.x,0.0)).xyz);
	const float gradientN = lumaN - lumaM;
	const float gradientS = lumaS - lumaM;
	const bool pairN = abs(gradientN) > abs(gradientS);
	float gradient = abs(gradientN);
	if(!pairN) gradient = abs(gradientS);
	const float gradientScaled = gradient * 0.25;
	float lumaNN = lumaN + lumaM;
	if(!pairN) lumaNN = lumaS + lumaM;
	const float lumaMN = lumaNN * 0.5;
	float lengthSign = offs.y;
	if(!horzSpan) lengthSign = offs.x;
	if( pairN) lengthSign = -lengthSign;
	float2 posN = u;
	if(!horzSpan) posN.x += lengthSign * 0.5;
	if( horzSpan) posN.y += lengthSign * 0.5;
	float2 posP = posN;
	float2 offNP = offs;
	if(!horzSpan) offNP.x = 0.0;
	if( horzSpan) offNP.y = 0.0;
	posP += offNP * FAAAOffMult[0];
	posN -= offNP * FAAAOffMult[0];
	float lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
	float lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
	lumaEndP -= lumaMN;
	lumaEndN -= lumaMN;
	bool doneP = abs(lumaEndP) > gradientScaled;
	bool doneN = abs(lumaEndN) > gradientScaled;
	int i = 0;
	UNROLL while(++i < FAAA_QUALITY__PI) {
		if(!doneP) {
			posP += offNP * FAAAOffMult[i];
			lumaEndP  = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
			lumaEndP -= lumaMN;
			doneP = abs(lumaEndP) > gradientScaled;
		}
		if(!doneN) {
			posN -= offNP * FAAAOffMult[i];
			lumaEndN  = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
			lumaEndN -= lumaMN;
			doneN = abs(lumaEndN) > gradientScaled;
		}
		if(doneN && doneP) break;
	}
	float2 posM = u;
	float dstN = posM.x - posN.x;
	float dstP = posP.x - posM.x;
	if(!horzSpan) dstN = posM.y - posN.y;
	if(!horzSpan) dstP = posP.y - posM.y;
	const bool dstNLTdstP = dstN < dstP;
	const bool lumaMLTZero = lumaM - lumaMN < 0.0;
	const bool mSpanLTZero = dstNLTdstP ? lumaEndN < 0.0 : lumaEndP < 0.0;
	const bool goodSpan = mSpanLTZero != lumaMLTZero;
	const float dst = dstNLTdstP ? dstN : dstP;
	const float spanLength = dstP + dstN;
	const float pixelOffset = (-dst / spanLength) + 0.5;
	const bool pixelOffsetLTZero = pixelOffset < 0.0;
	if(pixelOffsetLTZero || !goodSpan)
		return rgbyM;
	if(!horzSpan) posM.x += pixelOffset * lengthSign;
	if( horzSpan) posM.y += pixelOffset * lengthSign;
	return texStereoNoLod(tex_fb_filtered, posM).xyz;
}
