#include "hlsl_glsl.fxh"

#define PI 3.1415926535897932384626433832795

// GLSL is automatically defined by the framework
#ifndef GLSL
#define HLSL
#endif

#ifdef GLSL
#define texNoLod(tex, pos) textureLod(tex, pos, 0)
#else
#define texNoLod(tex, pos) tex2Dlod(tex, float4(pos, 0., 0.))
#endif

float sqr(const float v)
{
    return v*v;
}

float2 sqr(const float2 v)
{
    return v*v;
}

float3 sqr(const float3 v)
{
    return v*v;
}

float3 mul_w1(const float3 v, const float4x4 m)
{
    return v.x*m[0].xyz + (v.y*m[1].xyz + (v.z*m[2].xyz + m[3].xyz));
}

float3 mul_w1(const float3 v, const float4x3 m)
{
    return v.x*m[0] + (v.y*m[1] + (v.z*m[2] + m[3]));
}

float3 mul_w0(const float3 v, const float4x3 m)
{
    return v.x*m[0] + v.y*m[1] + v.z*m[2];
}

float acos_approx(const float v)
{
    const float x = abs(v);
    if(1. - x < 0.0000001) // necessary due to compiler doing 1./rsqrt instead of sqrt
       return (v >= 0.) ? 0. : PI;
    const float res = (-0.155972/*C1*/ * x + 1.56467/*C0*/) * sqrt(1. - x);
    return (v >= 0.) ? res : PI - res;
}

float acos_approx_divPI(const float v)
{
    //return acos(v)*(1./PI);

    const float x = abs(v);
    if(1. - x < 0.0000001) // necessary due to compiler doing 1./rsqrt instead of sqrt
       return (v >= 0.) ? 0. : 1.;
    const float res = ((-0.155972/PI)/*C1*/ * x + (1.56467/PI)/*C0*/) * sqrt(1. - x);
    return (v >= 0.) ? res : 1. - res;
}

float atan2_approx(const float y, const float x)
{
	const float abs_y = abs(y);
	const float abs_x = abs(x);

	if(abs_x < 0.0000001 && abs_y < 0.0000001)
	   return 0.;//(PI/4.);

	const float r = (abs_x - abs_y) / (abs_x + abs_y);
	const float angle = ((x < 0.) ? (0.75*PI) : (0.25*PI))
	                  + (0.211868/*C3*/ * r * r - 0.987305/*C1*/) * ((x < 0.) ? -r : r);
	return (y < 0.) ? -angle : angle;
}

float atan2_approx_div2PI(const float y, const float x)
{
	//return atan2(y,x)*(0.5/PI);

	const float abs_y = abs(y);
	const float abs_x = abs(x);

	if(abs_x < 0.0000001 && abs_y < 0.0000001)
	   return 0.;//(PI/4.)*(0.5/PI);

	const float r = (abs_x - abs_y) / (abs_x + abs_y);
	const float angle = ((x < 0.) ? (3./8.) : (1./8.))
	                  + (0.211868/*C3*//(2.*PI) * r * r - 0.987305/*C1*//(2.*PI)) * ((x < 0.) ? -r : r);
	return (y < 0.) ? -angle : angle;
}

#if 0
float asin_approx(const float v)
{
	return (0.5*PI) - acos_approx(v);
}

//!! this one is untested for special values (see atan2 approx!)
// 4th order hyperbolical approximation
// 7 * 10^-5 radians precision 
// Reference : Efficient approximations for the arctangent function, Rajan, S. Sichun Wang Inkol, R. Joyal, A., May 2006
float atan_approx(const float x)
{
	return x * (-0.1784 * abs(x) - 0.0663 * x*x + 1.0301);
}
#endif

//
// Gamma & ToneMapping
//

#define BURN_HIGHLIGHTS 0.25

#define mul(vecIn, MatIn) ((MatIn)*(vecIn))

float3 InvGamma(const float3 color) //!! use hardware support? D3DSAMP_SRGBTEXTURE
{
	return /*color * (color * (color * 0.305306011 + 0.682171111) + 0.012522878);/*/ pow(color, vec3(2.2)); // pow does not matter anymore on current GPUs
}

float3 InvToneMap(const float3 color)
{
	float inv_2bh = 0.5/BURN_HIGHLIGHTS;
	float bh = 4.0*BURN_HIGHLIGHTS - 2.0;
	return (color - 1.0 + sqrt(color*(color + bh) + 1.0))*inv_2bh;
}

float FBGamma(const float color) //!! use hardware support? D3DRS_SRGBWRITEENABLE
{
	return pow(color, 1.0/2.2); // pow does not matter anymore on current GPUs
}

vec2 FBGamma(const float2 color) //!! use hardware support? D3DRS_SRGBWRITEENABLE
{
	return pow(color, float2(1.0/2.2)); // pow does not matter anymore on current GPUs
}

float3 FBGamma(const float3 color) //!! use hardware support? D3DRS_SRGBWRITEENABLE
{
	return pow(color, float3(1.0/2.2)); // pow does not matter anymore on current GPUs

	/* float3 t0 = sqrt(color);
	float3 t1 = sqrt(t0);
	float3 t2 = sqrt(t1);
	return 0.662002687 * t0 + 0.684122060 * t1 - 0.323583601 * t2 - 0.0225411470 * color;*/
}

float3 FBToneMap(const float3 color)
{
    float l = dot(color,float3(0.176204,0.812985,0.0108109));
    return color * ((l*BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom   
}

#if 0
float3 FilmicToneMap(const float3 hdr, const float whitepoint) //!! test/experimental
{
    const float4 vh = float4(hdr,whitepoint);
    const float4 va = 1.425*vh + 0.05;
    const float4 vf = (vh*va + 0.004)/(vh*(va+0.55) + 0.0491) - 0.0821;
    return vf.rgb/vf.aaa;
}

float3 FilmicToneMap2(const float3 texColor) //!! test/experimental
{
    const float3 x = max(0.,texColor-0.004); // Filmic Curve
    return (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
}



// RGBM/RGBD

float3 DecodeRGBM(const float4 rgbm)
{
    return rgbm.rgb * (rgbm.a * MaxRange);
}

float4 EncodeRGBM(const float3 rgb)
{
    const float maxRGB = max(rgb.r, max(rgb.g, rgb.b));
    const float M = ceil(maxRGB * (255.0 / MaxRange)) * (1.0/255.0);
    return float4(rgb / (M * MaxRange), M); //!! handle rgb ~= 0 0 0
}

float3 DecodeRGBD(const float4 rgbd)
{
    return rgbd.rgb * ((MaxRange / 255.0) / rgbd.a);
}

float4 EncodeRGBD(const float3 rgb)
{
    const float maxRGB = max(rgb.r, max(rgb.g, rgb.b));
    const float D = saturate(floor(max(MaxRange / maxRGB, 1.0)) * (1.0/255.0)); //!! handle rgb ~= 0 0 0
    return float4(rgb * (D * (255.0 / MaxRange)), D);
}
#endif

//
// Blends
//

float4 Additive(const float4 cBase, const float4 cBlend, const float percent)
{
	return cBase + cBlend*percent;
}

float3 Screen (const float3 cBase, const float3 cBlend)
{
	return 1.0 - (1.0 - cBase) * (1.0 - cBlend);
}

float4 Screen (const float4 cBase, const float4 cBlend)
{
	return 1.0 - (1.0 - cBase) * (1.0 - cBlend);
}

float3 ScreenHDR (const float3 cBase, const float3 cBlend)
{
	return max(1.0 - (1.0 - cBase) * (1.0 - cBlend), float3(0.,0.,0.));
}

float4 ScreenHDR (const float4 cBase, const float4 cBlend)
{
	return max(1.0 - (1.0 - cBase) * (1.0 - cBlend), float4(0.,0.,0.,0.));
}

float4 Multiply (const float4 cBase, const float4 cBlend, const float percent)
{
	return cBase * cBlend*percent;
}

float4 Overlay (const float4 cBase, const float4 cBlend)
{
	// overlay has two output possbilities
	// which is taken is decided if pixel value
	// is below half or not

	float4 cNew = step(0.5, cBase);

	// we pick either solution
	// depending on pixel

	// first is case of < 0.5
	// second is case for >= 0.5

	// interpolate between the two, 
	// using color as influence value
	cNew = mix(cBase*cBlend*2.0, 1.0-2.0*(1.0-cBase)*(1.0-cBlend), cNew);

	//cNew.a = 1.0;
	return cNew;
}

float4 OverlayHDR (const float4 cBase, const float4 cBlend)
{
	// overlay has two output possbilities
	// which is taken is decided if pixel value
	// is below half or not

	float4 cNew = step(0.5, cBase);

	// we pick either solution
	// depending on pixel

	// first is case of < 0.5
	// second is case for >= 0.5

	// interpolate between the two, 
	// using color as influence value
	cNew = max(mix(cBase*cBlend*2.0, 1.0-2.0*(1.0-cBase)*(1.0-cBlend), cNew), float4(0.,0.,0.,0.));

	//cNew.a = 1.0;
	return cNew;
}
