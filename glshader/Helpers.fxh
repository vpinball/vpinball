#include "hlsl_glsl.fxh"

#define PI 3.1415926535897932384626433832795

float sqr( float v)
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

float acos_approx( float v)
{
	float x = abs(v);
	if(1.0 - x < 0.0000001) // necessary due to compiler doing 1./rsqrt instead of sqrt
	   return (v >= 0.) ? 0. : PI;
	float res = (-0.155972/*C1*/ * x + 1.56467/*C0*/) * sqrt(1.0 - x);
	return (v >= 0.) ? res : PI - res;
}

float acos_approx_divPI( float v)
{
	float x = abs(v);
	if(1.0 - x < 0.0000001) // necessary due to compiler doing 1./rsqrt instead of sqrt
	   return (v >= 0.) ? 0. : 1.;
	float res = ((-0.155972/PI)/*C1*/ * x + (1.56467/PI)/*C0*/) * sqrt(1.0 - x);
	return (v >= 0.) ? res : 1. - res;
}

float atan2_approx( float y,  float x)
{
	float abs_y = abs(y);
	float abs_x = abs(x);
	if(abs_x < 0.0000001 && abs_y < 0.0000001)
		return 0.;//(PI/4.);

	float r = (abs_x - abs_y) / (abs_x + abs_y);
	float angle = ((x < 0.) ? (0.75*PI) : (0.25*PI))
				  + (0.211868/*C3*/ * r * r - 0.987305/*C1*/) * ((x < 0.) ? -r : r);
	return (y < 0.) ? -angle : angle;
}

float atan2_approx_div2PI( float y,  float x)
{
	//return atan2(y,x)*(0.5/PI);

	float abs_y = abs(y);
	float abs_x = abs(x);

	if(abs_x < 0.0000001 && abs_y < 0.0000001)
		return 0.;//(PI/4.)*(0.5/PI);

	float r = (abs_x - abs_y) / (abs_x + abs_y);
	float angle = ((x < 0.) ? (3./8.) : (1./8.))
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

vec3 InvGamma( vec3 color) //!! use hardware support? D3DSAMP_SRGBTEXTURE
{
	return /*color * (color * (color * 0.305306011 + 0.682171111) + 0.012522878);/*/ pow(color, vec3(2.2)); // pow does not matter anymore on current GPUs
}

vec3 InvToneMap( vec3 color)
{
	float inv_2bh = 0.5/BURN_HIGHLIGHTS;
	float bh = 4.0*BURN_HIGHLIGHTS - 2.0;
	return (color - 1.0 + sqrt(color*(color + bh) + 1.0))*inv_2bh;
}

float FBGamma( float color) //!! use hardware support? D3DRS_SRGBWRITEENABLE
{
	return pow(color, 1.0/2.2); // pow does not matter anymore on current GPUs
}

vec2 FBGamma( vec2 color) //!! use hardware support? D3DRS_SRGBWRITEENABLE
{
	return pow(color, vec2(1.0/2.2)); // pow does not matter anymore on current GPUs
}

vec3 FBGamma( vec3 color) //!! use hardware support? D3DRS_SRGBWRITEENABLE
{
	return pow(color, vec3(1.0/2.2)); // pow does not matter anymore on current GPUs

	/* vec3 t0 = sqrt(color);
	vec3 t1 = sqrt(t0);
	vec3 t2 = sqrt(t1);
	return 0.662002687 * t0 + 0.684122060 * t1 - 0.323583601 * t2 - 0.0225411470 * color;*/
}

vec3 FBToneMap( vec3 color)
{
    float l = dot(color,vec3(0.176204,0.812985,0.0108109));
    return color * ((l*BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom   
}

#if 0
vec3 FilmicToneMap( vec3 hdr,  float whitepoint) //!! test/experimental
{
    vec4 vh = vec4(hdr,whitepoint);
    vec4 va = 1.425*vh + 0.05;
    vec4 vf = (vh*va + 0.004)/(vh*(va+0.55) + 0.0491) - 0.0821;
    return vf.rgb/vf.aaa;
}

vec3 FilmicToneMap2( vec3 texColor) //!! test/experimental
{
    vec3 x = max(0.,texColor-0.004); // Filmic Curve
    return (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
}



// RGBM/RGBD

vec3 DecodeRGBM( vec4 rgbm)
{
    return rgbm.rgb * (rgbm.a * MaxRange);
}

vec4 EncodeRGBM( vec3 rgb)
{
    float maxRGB = max(rgb.r, max(rgb.g, rgb.b));
    float M = ceil(maxRGB * (255.0 / MaxRange)) * (1.0/255.0);
    return vec4(rgb / (M * MaxRange), M); //!! handle rgb ~= 0 0 0
}

vec3 DecodeRGBD( vec4 rgbd)
{
    return rgbd.rgb * ((MaxRange / 255.0) / rgbd.a);
}

vec4 EncodeRGBD( vec3 rgb)
{
    float maxRGB = max(rgb.r, max(rgb.g, rgb.b));
    float D = saturate(floor(max(MaxRange / maxRGB, 1.0)) * (1.0/255.0)); //!! handle rgb ~= 0 0 0
    return vec4(rgb * (D * (255.0 / MaxRange)), D);
}
#endif

//
// Blends
//

vec4 Additive( vec4 cBase,  vec4 cBlend,  float percent)
{
   return cBase + cBlend*percent;
}

vec3 Screen ( vec3 cBase,  vec3 cBlend)
{
	return 1.0 - (1.0 - cBase) * (1.0 - cBlend);
}

vec4 Screen ( vec4 cBase,  vec4 cBlend)
{
	return 1.0 - (1.0 - cBase) * (1.0 - cBlend);
}

vec3 ScreenHDR ( vec3 cBase,  vec3 cBlend)
{
	return max(1.0 - (1.0 - cBase) * (1.0 - cBlend), vec3(0.,0.,0.));
}

vec4 ScreenHDR ( vec4 cBase,  vec4 cBlend)
{
	return max(1.0 - (1.0 - cBase) * (1.0 - cBlend), vec4(0.,0.,0.,0.));
}

vec4 Multiply ( vec4 cBase,  vec4 cBlend,  float percent)
{
	return cBase * cBlend*percent;
}

vec4 Overlay ( vec4 cBase,  vec4 cBlend)
{
	// overlay has two output possbilities
	// which is taken is decided if pixel value
	// is below half or not

	vec4 cNew = step(0.5, cBase);

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

vec4 OverlayHDR ( vec4 cBase,  vec4 cBlend)
{
	// overlay has two output possbilities
	// which is taken is decided if pixel value
	// is below half or not

	vec4 cNew = step(0.5, cBase);

	// we pick either solution
	// depending on pixel

	// first is case of < 0.5
	// second is case for >= 0.5

	// interpolate between the two, 
	// using color as influence value
	cNew = max(mix(cBase*cBlend*2.0, 1.0-2.0*(1.0-cBase)*(1.0-cBlend), cNew), vec4(0.,0.,0.,0.));

	//cNew.a = 1.0;
	return cNew;
}
