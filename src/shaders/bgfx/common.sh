// license:GPLv3+

#include "bgfx_shader.sh"

#define PI 3.1415926535897932384626433832795

// The selected min value is for mediump on GLES. Could be higher for other platforms but this may lead to rendering differences between platforms
// #define FLT_MIN_VALUE 0.0000001
#define FLT_MIN_VALUE 0.00006103515625

#if BGFX_SHADER_LANGUAGE_HLSL > 0 && BGFX_SHADER_LANGUAGE_HLSL < 400
#define DX9
#endif

#define texNoLod(tex, pos) texture2DLod(tex, pos, 0.0)

#ifdef STEREO
	#define SAMPLER2DSTEREO(_name, _reg) SAMPLER2DARRAY(_name, _reg)
	#define texStereo(tex, pos) texture2DArray(tex, vec3((pos).x, (pos).y, v_eye))
	#define texStereoLod(tex, pos, lod) texture2DArrayLod(tex, vec3((pos).x, (pos).y, v_eye), lod)
	#define N_EYES 2
#else
	#define SAMPLER2DSTEREO(_name, _reg) SAMPLER2D(_name, _reg)
	#define texStereo(tex, pos) texture2D(tex, pos)
	#define texStereoLod(tex, pos, lod) texture2DLod(tex, pos, lod)
	#define N_EYES 1
	#define v_eye 0.
#endif

#define texStereoNoLod(tex, pos) texStereoLod(tex, pos, 0.0)

#if BGFX_SHADER_LANGUAGE_GLSL || BGFX_SHADER_LANGUAGE_SPIRV
    // GLSL does not define the sincos function
    #define sincos(phi,sp,cp) {sp=sin(phi);cp=cos(phi);}
    #define const
#endif

float sqr(const float v)
{
    return v*v;
}

vec2 sqr(const vec2 v)
{
    return v*v;
}

vec3 sqr(const vec3 v)
{
    return v*v;
}

vec3 mul_w1(const vec3 v, const mat4 m)
{
    return v.x*m[0].xyz + (v.y*m[1].xyz + (v.z*m[2].xyz + m[3].xyz));
}

/*vec3 mul_w1(const vec3 v, const mat4x3 m)
{
    return v.x*m[0] + (v.y*m[1] + (v.z*m[2] + m[3]));
}

vec3 mul_w0(const vec3 v, const mat4x3 m)
{
    return v.x*m[0] + v.y*m[1] + v.z*m[2];
}*/

#if 0
float acos_approx(const float v)
{
    const float x = abs(v);
    if(1. - x <= FLT_MIN_VALUE) // necessary due to compiler doing 1./inversesqrt instead of sqrt
       return (v >= 0.) ? 0. : PI;
    const float res = (-0.155972/*C1*/ * x + 1.56467/*C0*/) * sqrt(1. - x);
    return (v >= 0.) ? res : PI - res;
}
#endif

float acos_approx_divPI(const float v)
{
    //return acos(v)*(1./PI);

    const float x = abs(v);
    if(1. - x <= FLT_MIN_VALUE) // necessary due to compiler doing 1./inversesqrt instead of sqrt
       return (v >= 0.) ? 0. : 1.;
    const float res = ((-0.155972/PI)/*C1*/ * x + (1.56467/PI)/*C0*/) * sqrt(1. - x);
    return (v >= 0.) ? res : 1. - res;
}

#if 0
float atan2_approx(const float y, const float x)
{
	const float abs_y = abs(y);
	const float abs_x = abs(x);

	if(abs_x <= FLT_MIN_VALUE && abs_y <= FLT_MIN_VALUE)
	   return 0.;//(PI/4.);

	const float r = (abs_x - abs_y) / (abs_x + abs_y);
	const float angle = ((x < 0.) ? (0.75*PI) : (0.25*PI))
	                  + (0.211868/*C3*/ * r * r - 0.987305/*C1*/) * ((x < 0.) ? -r : r);
	return (y < 0.) ? -angle : angle;
}
#endif

float atan2_approx_div2PI(const float y, const float x)
{
	//return atan2(y,x)*(0.5/PI);

	const float abs_y = abs(y);
	const float abs_x = abs(x);

	if(abs_x <= FLT_MIN_VALUE && abs_y <= FLT_MIN_VALUE)
	   return 0.;//(PI/4.)*(0.5/PI);

	const float r = (abs_x - abs_y) / (abs_x + abs_y);
	const float angle = ((x < 0.) ? (3./8.) : (1./8.))
	                  + (0.211868/*C3*//(2.*PI) * r * r - 0.987305/*C1*//(2.*PI)) * ((x < 0.) ? -r : r);
	return (y < 0.) ? -angle : angle;
}

#if 0
float asin_approx(const float v)
{
    const float x = abs(v);
    if(1. - x <= FLT_MIN_VALUE) // necessary due to compiler doing 1./inversesqrt instead of sqrt
       return (v >= 0.) ? (0.5*PI) : -(0.5*PI);
    const float res = (-0.155972/*C1*/ * x + 1.56467/*C0*/) * sqrt(1. - x);
    return (v >= 0.) ? (0.5*PI) - res : -(0.5*PI) + res;
}
#endif

float asin_approx_divPI(const float v)
{
    //return asin(v) / PI;

    const float x = abs(v);
    if(1. - x <= FLT_MIN_VALUE) // necessary due to compiler doing 1./inversesqrt instead of sqrt
       return (v >= 0.) ? 0.5 : -0.5;
    const float res = ((-0.155972/PI)/*C1*/ * x + (1.56467/PI)/*C0*/) * sqrt(1. - x);
    return (v >= 0.) ? 0.5 - res : -0.5 + res;
}

vec2 ray_to_equirectangular_uv(const vec3 ray)
{
   return vec2( // remap to 2D envmap coords
		0.5 + atan2_approx_div2PI(ray.y, ray.x),
		acos_approx_divPI(ray.z));
}

#if 0
//!! this one is untested for special values (see atan2 approx!)
// 4th order hyperbolical approximation
// 7 * 10^-5 radians precision 
// Reference : Efficient approximations for the arctangent function, Rajan, S. Sichun Wang Inkol, R. Joyal, A., May 2006
float atan_approx(const float x)
{
	return x * (-0.1784 * abs(x) - 0.0663 * x*x + 1.0301);
}

mat4x4 inverse4x4(const mat4x4 m)
{
   float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
   float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
   float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
   float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

   float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
   float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
   float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
   float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

   float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
   float idet = 1.0 / det;

   vec4x4 ret;

   ret[0][0] = t11 * idet;
   ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
   ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
   ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

   ret[1][0] = t12 * idet;
   ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
   ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
   ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

   ret[2][0] = t13 * idet;
   ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
   ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
   ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

   ret[3][0] = t14 * idet;
   ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
   ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
   ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

   return ret;
}
#endif


//
// Gamma & ToneMapping
//

#define BURN_HIGHLIGHTS 0.25

float InvsRGB(const float color)
{
    //return /*color * (color * (color * 0.305306011 + 0.682171111) + 0.012522878);/*/ pow(color, 2.2); // pow does not matter anymore on current GPUs //!! not completely true for example when tested with FSS tables

    if (color <= 0.04045) // 0.03928 ?
        return color * (1.0/12.92);
    else
        return pow(color * (1.0/1.055) + (0.055/1.055), 2.4);
}

float InvRec709(const float color)
{
    if (color <= 0.081)
        return color * (1.0/4.5);
    else
        return pow(color * (1.0/1.099) + (0.099/1.099), 1.0/0.45);
}

float InvGamma(const float color) //!! use hardware support? D3DSAMP_SRGBTEXTURE
{
    return InvsRGB(color);
}

vec3 InvGamma(const vec3 color) //!! use hardware support? D3DSAMP_SRGBTEXTURE
{
    return vec3(InvGamma(color.x),InvGamma(color.y),InvGamma(color.z));
}

vec3 InvToneMap(const vec3 color)
{
    const float inv_2bh = 0.5/BURN_HIGHLIGHTS;
    const float bh = 4.0*BURN_HIGHLIGHTS - 2.0;
    return (color - 1.0 + sqrt(color*(color + bh) + 1.0))*inv_2bh;
}

#if 1
float sRGB(const float f)
{
    //return pow(f, 1.0/2.2);
    // pow does not matter anymore on current GPUs (tested also on tablet/intel)
    /*const float t0 = sqrt(f);
    const float t1 = sqrt(t0);
    const float t2 = sqrt(t1);
    return 0.662002687 * t0 + 0.684122060 * t1 - 0.323583601 * t2 - 0.0225411470 * f;*/

    float s;
    //if (!(f > 0.0)) // also covers NaNs
    //    s = 0.0;
    /*else*/ if (f <= 0.0031308)
        s = 12.92 * f;
    else //if (f < 1.0)
        s = 1.055 * pow(f, 1.0/2.4) - 0.055;
    //else
    //    s = 1.0;

    return s;
}

float Rec709(const float f)
{
    float s;
    //if (!(f > 0.0)) // also covers NaNs
    //    s = 0.0;
    /*else*/ if (f <= 0.018)
        s = 4.5 * f;
    else //if (f < 1.0)
        s = 1.099 * pow(f, 0.45) - 0.099;
    //else
    //    s = 1.0;

    return s;
}

float FBGamma(const float color)
{
    return sRGB(color);
}
vec2 FBGamma(const vec2 color)
{
    return vec2(FBGamma(color.x),FBGamma(color.y));
}
vec3 FBGamma(const vec3 color)
{
    return vec3(FBGamma(color.x),FBGamma(color.y),FBGamma(color.z));
}
#else
#define FBGamma // uses hardware support via D3DRS_SRGBWRITEENABLE
#endif

//#define FBGamma22(color) pow(color, 1.0/2.2) // apparently most consumer displays nowadays feature a plain 2.2 curve
vec3 FBGamma22(const vec3 color)
{
    return vec3(pow(color.x, 1.0/2.2),pow(color.y, 1.0/2.2),pow(color.z, 1.0/2.2));
}

float FBToneMap(const float l)
{
    return l * ((l*BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
vec2 FBToneMap(const vec2 color)
{
    const float l = dot(color,vec2(0.176204+0.0108109*0.5,0.812985+0.0108109*0.5));
    return color * ((l*BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}
vec3 FBToneMap(const vec3 color)
{
    const float l = dot(color,vec3(0.176204,0.812985,0.0108109));
    return color * ((l*BURN_HIGHLIGHTS + 1.0) / (l + 1.0)); // overflow is handled by bloom
}

#if 0
vec3 FilmicToneMap(const vec3 hdr, const float whitepoint) //!! test/experimental
{
    const vec4 vh = vec4(hdr,whitepoint);
    const vec4 va = 1.425*vh + 0.05;
    const vec4 vf = (vh*va + 0.004)/(vh*(va+0.55) + 0.0491) - 0.0821;
    return vf.rgb/vf.aaa;
}

vec3 FilmicToneMap2(const vec3 texColor) //!! test/experimental
{
    const vec3 x = max(0.,texColor-0.004); // Filmic Curve
    return (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);
}



// RGBM/RGBD

vec3 DecodeRGBM(const vec4 rgbm)
{
    return rgbm.rgb * (rgbm.a * MaxRange);
}

vec4 EncodeRGBM(const vec3 rgb)
{
    const float maxRGB = max(rgb.r, max(rgb.g, rgb.b));
    const float M = ceil(maxRGB * (255.0 / MaxRange)) * (1.0/255.0);
    return vec4(rgb / (M * MaxRange), M); //!! handle rgb ~= 0 0 0
}

vec3 DecodeRGBD(const vec4 rgbd)
{
    return rgbd.rgb * ((MaxRange / 255.0) / rgbd.a);
}

vec4 EncodeRGBD(const vec3 rgb)
{
    const float maxRGB = max(rgb.r, max(rgb.g, rgb.b));
    const float D = saturate(floor(max(MaxRange / maxRGB, 1.0)) * (1.0/255.0)); //!! handle rgb ~= 0 0 0
    return vec4(rgb * (D * (255.0 / MaxRange)), D);
}
#endif

//
// Blends
//

vec4 Additive(const vec4 cBase, const vec4 cBlend, const float percent)
{
	return cBase + cBlend*percent;
}

vec3 Screen (const vec3 cBase, const vec3 cBlend)
{
	return 1.0 - (1.0 - cBase) * (1.0 - cBlend);
}

vec4 Screen (const vec4 cBase, const vec4 cBlend)
{
	return 1.0 - (1.0 - cBase) * (1.0 - cBlend);
}

vec3 ScreenHDR (const vec3 cBase, const vec3 cBlend)
{
	return max(1.0 - (1.0 - cBase) * (1.0 - cBlend), vec3(0.,0.,0.));
}

vec4 ScreenHDR (const vec4 cBase, const vec4 cBlend)
{
	return max(1.0 - (1.0 - cBase) * (1.0 - cBlend), vec4(0.,0.,0.,0.));
}

vec4 Multiply (const vec4 cBase, const vec4 cBlend, const float percent)
{
	return cBase * cBlend*percent;
}

vec4 Overlay (const vec4 cBase, const vec4 cBlend)
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

vec4 OverlayHDR (const vec4 cBase, const vec4 cBlend)
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

//vec3 sphere_sample(const vec2 t)
//{
//const float phi = t.y * (2.0*3.1415926535897932384626433832795);
//const float z = 1.0 - t.x*2.0;
//const float r = sqrt(1.0 - z*z);
//float sp,cp;
//sincos(phi,sp,cp);
//return vec3(cp*r, z, sp*r);
//}

vec3 cos_hemisphere_sample(const vec2 t) // u,v in [0..1), returns y-up
{
	const float phi = t.y * (2.0*3.1415926535897932384626433832795);
	const float cosTheta = sqrt(1.0 - t.x);
	const float sinTheta = sqrt(t.x);
	float sp,cp;
	sincos(phi,sp,cp);
	return vec3(cp * sinTheta, cosTheta, sp * sinTheta);
}

vec3 rotate_to_vector_upper(const vec3 vec, const vec3 normal)
{
	if(normal.y > -0.99999)
	{
		const float h = 1.0/(1.0+normal.y);
		const float hz = h*normal.z;
		const float hzx = hz*normal.x;
		return vec3(
			vec.x * (normal.y+hz*normal.z) + vec.y * normal.x - vec.z * hzx,
			vec.y * normal.y - vec.x * normal.x - vec.z * normal.z,
			vec.y * normal.z - vec.x * hzx + vec.z * (normal.y+h*normal.x*normal.x));
	}
	else return -vec;
}

/*vec3 cos_hemisphere_sample(const vec3 normal, vec2 uv)
{
	const float theta = (2.0*3.1415926535897932384626433832795) * uv.x;
	uv.y = 2.0 * uv.y - 1.0;
	float st,ct;
	sincos(theta,st,ct);
	const vec3 spherePoint = vec3(sqrt(1.0 - uv.y * uv.y) * vec2(ct, st), uv.y);
	return normalize(normal + spherePoint);
}*/

/*vec3 cos_hemisphere_nonunit_sample(const vec3 normal, vec2 uv)
{
	const float theta = (2.0*3.1415926535897932384626433832795) * uv.x;
	uv.y = 2.0 * uv.y - 1.0;
	float st,ct;
	sincos(theta,st,ct);
	const vec3 spherePoint = vec3(sqrt(1.0 - uv.y * uv.y) * vec2(ct, st), uv.y);
	return normal + spherePoint;
}*/
