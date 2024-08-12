// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"


// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

SAMPLER2DSTEREO(tex_fb_filtered,  0); // Framebuffer (filtered)
SAMPLER2DSTEREO(tex_depth,        4); // DepthBuffer

// Some of the texFetch are unfiltered, but since they are aligned ot exact texel coords, this does not need a dedicated sampler
#define tex_fb_unfiltered tex_fb_filtered


float luma(const vec3 l)
{
    return dot(l, vec3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
    //return 0.299*l.x + 0.587*l.y + 0.114*l.z;
    //return 0.2126*l.x + 0.7152*l.y + 0.0722*l.z; // photometric
    //return sqrt(0.299 * l.x*l.x + 0.587 * l.y*l.y + 0.114 * l.z*l.z); // hsp
}

// Approximation of FXAA
#ifdef STEREO
vec4 fxaa1(const vec2 u, const float v_eye)
#else
vec4 fxaa1(const vec2 u)
#endif
{
	const vec3 rMc = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
			return vec4(rMc, 1.0);
	}

	const vec2 offs = w_h_height.xy;
	const float rNW = luma(texStereoNoLod(tex_fb_unfiltered, u - offs).xyz);
	const float rN = luma(texStereoNoLod(tex_fb_unfiltered, u - vec2(0.0,offs.y)).xyz);
	const float rNE = luma(texStereoNoLod(tex_fb_unfiltered, u - vec2(-offs.x,offs.y)).xyz);
	const float rW = luma(texStereoNoLod(tex_fb_unfiltered, u - vec2(offs.x,0.0)).xyz);
	const float rM = luma(rMc);
	const float rE = luma(texStereoNoLod(tex_fb_unfiltered, u + vec2(offs.x,0.0)).xyz);
	const float rSW = luma(texStereoNoLod(tex_fb_unfiltered, u + vec2(-offs.x,offs.y)).xyz);
	const float rS = luma(texStereoNoLod(tex_fb_unfiltered, u + vec2(0.0,offs.y)).xyz);
	const float rSE = luma(texStereoNoLod(tex_fb_unfiltered, u + offs).xyz);
	const float rMrN = rM+rN;
	const float lumaNW = rMrN+rNW+rW;
	const float lumaNE = rMrN+rNE+rE;
	const float rMrS = rM+rS;
	const float lumaSW = rMrS+rSW+rW;
	const float lumaSE = rMrS+rSE+rE;
	const bool g0 = (lumaSW > lumaSE);
	const float tempMax = g0 ? lumaSW : lumaSE;
	const float tempMin = g0 ? lumaSE : lumaSW;
	const bool g1 = (lumaNW > lumaNE);
	const float tempMax2 = g1 ? lumaNW : lumaNE;
	const float tempMin2 = g1 ? lumaNE : lumaNW;
	const float lumaMin = min(rM, min(tempMin, tempMin2));
	const float lumaMax = max(rM, max(tempMax, tempMax2));
	const float SWSE = lumaSW + lumaSE;
	const float NWNE = lumaNW + lumaNE;
	vec2 dir = vec2(SWSE - NWNE, (lumaNW + lumaSW) - (lumaNE + lumaSE));
	const float temp = 1.0/(min(abs(dir.x), abs(dir.y)) + max((NWNE + SWSE)*0.03125, 0.0078125)); //!! tweak?
	dir = clamp(dir*temp, vec2_splat(-8.0), vec2_splat(8.0)) * offs; //!! tweak?
	const vec3 rgbA = 0.5 * (texStereoNoLod(tex_fb_filtered, u-dir*(0.5/3.0)).xyz + texStereoNoLod(tex_fb_filtered, u+dir*(0.5/3.0)).xyz);
	const vec3 rgbB = 0.5 * rgbA + 0.25 * (texStereoNoLod(tex_fb_filtered, u-dir*0.5).xyz + texStereoNoLod(tex_fb_filtered, u+dir*0.5).xyz);
	const float lumaB = luma(rgbB);
	return vec4(((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB, 1.0);
}

#define FXAA_QUALITY_P0 1.5
#define FXAA_QUALITY_P1 2.0
#define FXAA_QUALITY_P2 8.0

// Full mid-quality PC FXAA 3.11
#ifdef STEREO
vec4 fxaa2(const vec2 u, const float v_eye)
#else
vec4 fxaa2(const vec2 u)
#endif
{
	const vec3 rgbyM = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
			return vec4(rgbyM, 1.0);
	}

	const vec2 offs = w_h_height.xy;
	const float lumaNW = luma(texStereoNoLod(tex_fb_unfiltered, u - offs).xyz);
	float lumaN = luma(texStereoNoLod(tex_fb_unfiltered, u - vec2(0.0,offs.y)).xyz);
	const float lumaNE = luma(texStereoNoLod(tex_fb_unfiltered, u - vec2(-offs.x,offs.y)).xyz);
	const float lumaW = luma(texStereoNoLod(tex_fb_unfiltered, u - vec2(offs.x,0.0)).xyz);
	const float lumaM = luma(rgbyM);
	const float lumaE = luma(texStereoNoLod(tex_fb_unfiltered, u + vec2(offs.x,0.0)).xyz);
	const float lumaSW = luma(texStereoNoLod(tex_fb_unfiltered, u + vec2(-offs.x,offs.y)).xyz);
	float lumaS = luma(texStereoNoLod(tex_fb_unfiltered, u + vec2(0.0,offs.y)).xyz);
	const float lumaSE = luma(texStereoNoLod(tex_fb_unfiltered, u + offs).xyz);
	const float maxSM = max(lumaS, lumaM);
	const float minSM = min(lumaS, lumaM);
	const float maxESM = max(lumaE, maxSM);
	const float minESM = min(lumaE, minSM);
	const float maxWN = max(lumaN, lumaW);
	const float minWN = min(lumaN, lumaW);
	const float rangeMax = max(maxWN, maxESM);
	const float rangeMin = min(minWN, minESM);
	const float rangeMaxScaled = rangeMax * 0.125; //0.333 (faster) .. 0.063 (slower) // reshade: 0.125, fxaa : 0.166
	const float range = rangeMax - rangeMin;
	const float rangeMaxClamped = max(0.0833, rangeMaxScaled); //0.0625 (high quality/faster) .. 0.0312 (visible limit/slower) // reshade: 0.0, fxaa : 0.0833
	const bool earlyExit = range < rangeMaxClamped;
	BRANCH if(earlyExit)
		return vec4(rgbyM, 1.0);
	const float lumaNS = lumaN + lumaS;
	const float lumaWE = lumaW + lumaE;
	const float subpixRcpRange = 1.0/range;
	const float subpixNSWE = lumaNS + lumaWE;
	const float edgeHorz1 = -2.0 * lumaM + lumaNS;
	const float edgeVert1 = -2.0 * lumaM + lumaWE;
	const float lumaNESE = lumaNE + lumaSE;
	const float lumaNWNE = lumaNW + lumaNE;
	const float edgeHorz2 = -2.0 * lumaE + lumaNESE;
	const float edgeVert2 = -2.0 * lumaN + lumaNWNE;
	const float lumaNWSW = lumaNW + lumaSW;
	const float lumaSWSE = lumaSW + lumaSE;
	const float edgeHorz4 = abs(edgeHorz1) * 2.0 + abs(edgeHorz2);
	const float edgeVert4 = abs(edgeVert1) * 2.0 + abs(edgeVert2);
	const float edgeHorz3 = -2.0 * lumaW + lumaNWSW;
	const float edgeVert3 = -2.0 * lumaS + lumaSWSE;
	const float edgeHorz = abs(edgeHorz3) + edgeHorz4;
	const float edgeVert = abs(edgeVert3) + edgeVert4;
	const float subpixNWSWNESE = lumaNWSW + lumaNESE;
	float lengthSign = offs.x;
	const bool horzSpan = edgeHorz >= edgeVert;
	const float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
	if(!horzSpan) lumaN = lumaW;
	if(!horzSpan) lumaS = lumaE;
	if(horzSpan) lengthSign = offs.y;
	const float subpixB = subpixA * (1.0/12.0) - lumaM;
	const float gradientN = lumaN - lumaM;
	const float gradientS = lumaS - lumaM;
	float lumaNN = lumaN + lumaM;
	const float lumaSS = lumaS + lumaM;
	const bool pairN = (abs(gradientN) >= abs(gradientS));
	const float gradient = max(abs(gradientN), abs(gradientS));
	if(pairN) lengthSign = -lengthSign;
	const float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);
	const vec2 offNP = vec2(!horzSpan ? 0.0 : offs.x, horzSpan ? 0.0 : offs.y);
	vec2 posB = u;
	const float l05 = lengthSign * 0.5;
	if(horzSpan) posB.y += l05;
	else posB.x += l05;
	vec2 posN = vec2(posB.x - offNP.x * FXAA_QUALITY_P0, posB.y - offNP.y * FXAA_QUALITY_P0);
	vec2 posP = vec2(posB.x + offNP.x * FXAA_QUALITY_P0, posB.y + offNP.y * FXAA_QUALITY_P0);
	const float subpixD = -2.0 * subpixC + 3.0;
	float lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
	const float subpixE = subpixC * subpixC;
	float lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
	if(!pairN) lumaNN = lumaSS;
	const float gradientScaled = gradient * (1.0/4.0);
	const float lumaMM = lumaM - lumaNN * 0.5;
	const float subpixF = subpixD * subpixE;
	const bool lumaMLTZero = (lumaMM < 0.0);
	lumaEndN -= lumaNN * 0.5;
	lumaEndP -= lumaNN * 0.5;
	bool doneN = (abs(lumaEndN) >= gradientScaled);
	bool doneP = (abs(lumaEndP) >= gradientScaled);
	if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P1;
	if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P1;
	bool doneNP = ((!doneN) || (!doneP));
	if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P1;
	if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P1;
	if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P2;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P2;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P2;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P2;
	}
	const float dstN = horzSpan ? (u.x - posN.x) : (u.y - posN.y);
	const float dstP = horzSpan ? (posP.x - u.x) : (posP.y - u.y);
	const bool goodSpanN = ((lumaEndN < 0.0) != lumaMLTZero);
	const float spanLength = dstP + dstN;
	const bool goodSpanP = ((lumaEndP < 0.0) != lumaMLTZero);
	const float spanLengthRcp = 1.0/spanLength;
	const bool directionN = (dstN < dstP);
	const float dst = min(dstN, dstP);
	const bool goodSpan = directionN ? goodSpanN : goodSpanP;
	const float subpixG = subpixF * subpixF;
	const float pixelOffset = 0.5 - dst * spanLengthRcp;
	const float subpixH = subpixG * 0.5; //1.00 (upper limit/softer) .. 0.50 (lower limit/sharper) .. 0.00 (completely off) // reshade : 0.25, fxaa : 0.75
	const float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
	const float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
	vec2 un = u;
	const float pl = pixelOffsetSubpix * lengthSign;
	if(horzSpan) un.y += pl;
	else un.x += pl;
	return vec4(texStereoNoLod(tex_fb_filtered, un).xyz, 1.0);
}

#undef FXAA_QUALITY_P0
#undef FXAA_QUALITY_P1
#undef FXAA_QUALITY_P2

#define FXAA_QUALITY_P0 1.0
#define FXAA_QUALITY_P1 1.0
#define FXAA_QUALITY_P2 1.0
#define FXAA_QUALITY_P3 1.0
#define FXAA_QUALITY_P4 1.0
#define FXAA_QUALITY_P5 1.5
#define FXAA_QUALITY_P6 2.0
#define FXAA_QUALITY_P7 2.0
#define FXAA_QUALITY_P8 2.0
#define FXAA_QUALITY_P9 2.0
#define FXAA_QUALITY_P10 4.0
#define FXAA_QUALITY_P11 8.0

// Full extreme-quality PC FXAA 3.11
#ifdef STEREO
vec4 fxaa3(const vec2 u, const float v_eye)
#else
vec4 fxaa3(const vec2 u)
#endif
{
	const vec3 rgbyM = texStereoNoLod(tex_fb_unfiltered, u).xyz;
	BRANCH if(w_h_height.w == 1.0) // depth buffer available?
	{
		const float depth0 = texStereoNoLod(tex_depth, u).x;
		BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
			return vec4(rgbyM, 1.0);
	}

	const vec2 offs = w_h_height.xy;
	const float lumaNW = luma(texStereoNoLod(tex_fb_unfiltered, u - offs).xyz);
	float lumaN = luma(texStereoNoLod(tex_fb_unfiltered, u - vec2(0.0,offs.y)).xyz);
	const float lumaNE = luma(texStereoNoLod(tex_fb_unfiltered, u - vec2(-offs.x,offs.y)).xyz);
	const float lumaW = luma(texStereoNoLod(tex_fb_unfiltered, u - vec2(offs.x,0.0)).xyz);
	const float lumaM = luma(rgbyM);
	const float lumaE = luma(texStereoNoLod(tex_fb_unfiltered, u + vec2(offs.x,0.0)).xyz);
	const float lumaSW = luma(texStereoNoLod(tex_fb_unfiltered, u + vec2(-offs.x,offs.y)).xyz);
	float lumaS = luma(texStereoNoLod(tex_fb_unfiltered, u + vec2(0.0,offs.y)).xyz);
	const float lumaSE = luma(texStereoNoLod(tex_fb_unfiltered, u + offs).xyz);
	const float maxSM = max(lumaS, lumaM);
	const float minSM = min(lumaS, lumaM);
	const float maxESM = max(lumaE, maxSM);
	const float minESM = min(lumaE, minSM);
	const float maxWN = max(lumaN, lumaW);
	const float minWN = min(lumaN, lumaW);
	const float rangeMax = max(maxWN, maxESM);
	const float rangeMin = min(minWN, minESM);
	const float rangeMaxScaled = rangeMax * 0.125; //0.333 (faster) .. 0.063 (slower) // reshade: 0.125, fxaa : 0.166
	const float range = rangeMax - rangeMin;
	const float rangeMaxClamped = max(0.0833, rangeMaxScaled); //0.0625 (high quality/faster) .. 0.0312 (visible limit/slower) // reshade: 0.0, fxaa : 0.0833
	const bool earlyExit = range < rangeMaxClamped;
	BRANCH if(earlyExit)
		return vec4(rgbyM, 1.0);
	const float lumaNS = lumaN + lumaS;
	const float lumaWE = lumaW + lumaE;
	const float subpixRcpRange = 1.0/range;
	const float subpixNSWE = lumaNS + lumaWE;
	const float edgeHorz1 = -2.0 * lumaM + lumaNS;
	const float edgeVert1 = -2.0 * lumaM + lumaWE;
	const float lumaNESE = lumaNE + lumaSE;
	const float lumaNWNE = lumaNW + lumaNE;
	const float edgeHorz2 = -2.0 * lumaE + lumaNESE;
	const float edgeVert2 = -2.0 * lumaN + lumaNWNE;
	const float lumaNWSW = lumaNW + lumaSW;
	const float lumaSWSE = lumaSW + lumaSE;
	const float edgeHorz4 = abs(edgeHorz1) * 2.0 + abs(edgeHorz2);
	const float edgeVert4 = abs(edgeVert1) * 2.0 + abs(edgeVert2);
	const float edgeHorz3 = -2.0 * lumaW + lumaNWSW;
	const float edgeVert3 = -2.0 * lumaS + lumaSWSE;
	const float edgeHorz = abs(edgeHorz3) + edgeHorz4;
	const float edgeVert = abs(edgeVert3) + edgeVert4;
	const float subpixNWSWNESE = lumaNWSW + lumaNESE;
	float lengthSign = offs.x;
	const bool horzSpan = edgeHorz >= edgeVert;
	const float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
	if(!horzSpan) lumaN = lumaW;
	if(!horzSpan) lumaS = lumaE;
	if(horzSpan) lengthSign = offs.y;
	const float subpixB = subpixA * (1.0/12.0) - lumaM;
	const float gradientN = lumaN - lumaM;
	const float gradientS = lumaS - lumaM;
	float lumaNN = lumaN + lumaM;
	const float lumaSS = lumaS + lumaM;
	const bool pairN = (abs(gradientN) >= abs(gradientS));
	const float gradient = max(abs(gradientN), abs(gradientS));
	if(pairN) lengthSign = -lengthSign;
	const float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0.0, 1.0);
	const vec2 offNP = vec2(!horzSpan ? 0.0 : offs.x, horzSpan ? 0.0 : offs.y);
	vec2 posB = u;
	const float l05 = lengthSign * 0.5;
	if(horzSpan) posB.y += l05;
	else posB.x += l05;
	vec2 posN = vec2(posB.x - offNP.x * FXAA_QUALITY_P0, posB.y - offNP.y * FXAA_QUALITY_P0);
	vec2 posP = vec2(posB.x + offNP.x * FXAA_QUALITY_P0, posB.y + offNP.y * FXAA_QUALITY_P0);
	const float subpixD = -2.0 * subpixC + 3.0;
	float lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
	const float subpixE = subpixC * subpixC;
	float lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
	if(!pairN) lumaNN = lumaSS;
	const float gradientScaled = gradient * (1.0/4.0);
	const float lumaMM = lumaM - lumaNN * 0.5;
	const float subpixF = subpixD * subpixE;
	const bool lumaMLTZero = (lumaMM < 0.0);
	lumaEndN -= lumaNN * 0.5;
	lumaEndP -= lumaNN * 0.5;
	bool doneN = (abs(lumaEndN) >= gradientScaled);
	bool doneP = (abs(lumaEndP) >= gradientScaled);
	if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P1;
	if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P1;
	bool doneNP = ((!doneN) || (!doneP));
	if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P1;
	if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P1;
	BRANCH if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P2;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P2;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P2;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P2;

		//

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P3;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P3;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P3;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P3;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P4;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P4;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P4;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P4;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P5;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P5;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P5;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P5;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P6;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P6;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P6;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P6;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P7;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P7;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P7;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P7;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P8;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P8;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P8;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P8;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P9;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P9;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P9;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P9;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P10;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P10;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P10;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P10;

		if(doneNP) {
		if(!doneN) lumaEndN = luma(texStereoNoLod(tex_fb_filtered, posN).xyz);
		if(!doneP) lumaEndP = luma(texStereoNoLod(tex_fb_filtered, posP).xyz);
		if(!doneN) lumaEndN = lumaEndN - lumaNN * 0.5;
		if(!doneP) lumaEndP = lumaEndP - lumaNN * 0.5;
		doneN = abs(lumaEndN) >= gradientScaled;
		doneP = abs(lumaEndP) >= gradientScaled;
		if(!doneN) posN.x -= offNP.x * FXAA_QUALITY_P11;
		if(!doneN) posN.y -= offNP.y * FXAA_QUALITY_P11;
		doneNP = ((!doneN) || (!doneP));
		if(!doneP) posP.x += offNP.x * FXAA_QUALITY_P11;
		if(!doneP) posP.y += offNP.y * FXAA_QUALITY_P11;
		}
		}
		}
		}
		}
		}
		}
		}
		}
	}
	const float dstN = horzSpan ? (u.x - posN.x) : (u.y - posN.y);
	const float dstP = horzSpan ? (posP.x - u.x) : (posP.y - u.y);
	const bool goodSpanN = ((lumaEndN < 0.0) != lumaMLTZero);
	const float spanLength = dstP + dstN;
	const bool goodSpanP = ((lumaEndP < 0.0) != lumaMLTZero);
	const float spanLengthRcp = 1.0/spanLength;
	const bool directionN = (dstN < dstP);
	const float dst = min(dstN, dstP);
	const bool goodSpan = directionN ? goodSpanN : goodSpanP;
	const float subpixG = subpixF * subpixF;
	const float pixelOffset = 0.5 - dst * spanLengthRcp;
	const float subpixH = subpixG * 0.5; //1.00 (upper limit/softer) .. 0.50 (lower limit/sharper) .. 0.00 (completely off) // reshade : 0.25, fxaa : 0.75
	const float pixelOffsetGood = goodSpan ? pixelOffset : 0.0;
	const float pixelOffsetSubpix = max(pixelOffsetGood, subpixH);
	vec2 un = u;
	const float pl = pixelOffsetSubpix * lengthSign;
	if(horzSpan) un.y += pl;
	else un.x += pl;
	return vec4(texStereoNoLod(tex_fb_filtered, un).xyz, 1.0);
}


void main()
{
#ifdef STEREO
#if defined(FXAA1)
	gl_FragColor = fxaa1(v_texcoord0, v_eye);
#elif defined(FXAA2)
	gl_FragColor = fxaa2(v_texcoord0, v_eye);
#elif defined(FXAA3)
	gl_FragColor = fxaa3(v_texcoord0, v_eye);
#endif
#else
#if defined(FXAA1)
	gl_FragColor = fxaa1(v_texcoord0);
#elif defined(FXAA2)
	gl_FragColor = fxaa2(v_texcoord0);
#elif defined(FXAA3)
	gl_FragColor = fxaa3(v_texcoord0);
#endif
#endif
}
