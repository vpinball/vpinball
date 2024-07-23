////GLOBAL

//**************************************************************************
// GLSL defines to support common shader code
#ifdef GLSL
    #ifdef SHADER_GLES30 // OpenGL ES (for standalone builds)
        #define FLT_MIN_VALUE 0.00006103515625
        precision highp float;
        #define noperspective
        #define flat
        #define VS_CLIP_DISTANCE(pos) 
    #else
        #define FLT_MIN_VALUE 0.0000001
        #define VS_CLIP_DISTANCE(pos) gl_ClipDistance[0] = dot(pos, clip_plane)
    #endif

    //HLSL to GLSL helpers
    #define ddx(x) dFdx(x)
    #define ddy(x) dFdy(x)
    #define lerp(x,y,s) mix(x,y,s)
    #define saturate(x) clamp(x,0.0,1.0)
    #define any(x) (dot(x,x)>0.0005)
    #define sincos(phi,sp,cp) {sp=sin(phi);cp=cos(phi);}
    #define frac(x) fract(x)
    #define rsqrt(x) inversesqrt(x)
    #define rcp(x) (1.0/(x))

    #define mul(vecIn, MatIn) ((MatIn)*(vecIn))

    #define tex2Dlod(sampler, v) textureLod(sampler, (v).xy,(v).w)
    #define tex2D(sampler, v) texture(sampler, v)

    #define const
    #define UNIFORM uniform

    #define float4 vec4
    #define float3 vec3
    #define float2 vec2

    #define float4x4 mat4
    #define float3x3 mat3
    #define float2x2 mat2

    #define float4x3 mat4x3
    #define float3x4 mat3x4

    #define to_float3x3 mat3
    #define MAT3_BEGIN mat3(
    #define MAT_END )
    #define MAT_ROW3_BEGIN vec3(
    #define MAT_ROW_END )

    //Not working :-(
    //vec4 pow(vec4 b, float e) {return pow(b,vec4(e));}
    //vec3 pow(vec3 b, float e) {return pow(b,vec3(e));}
    //vec2 pow(vec2 b, float e) {return pow(b,vec2(e));}

    #define texNoLod(tex, pos) textureLod(tex, pos, 0.)
    #define BRANCH
    #define UNROLL

    #if (N_EYES == 1)
        #define samplerStereo sampler2D
        #define FS_LAYER_TO_GLOBAL
        #define texStereo(tex, pos) texture(tex, pos)
        #define texStereolod(tex, pos) textureLod(tex, (pos).xy, (pos).w)
        #define texStereoNoLod(tex, pos) textureLod(tex, pos, 0.)
        #define texStereoOffset(tex, pos, offset) textureOffset(tex, pos, offset)
        #define texStereoOffsetNoLod(tex, pos, offset) textureLodOffset(tex, pos, 0.0, offset)
        #define texStereoGather(tex, pos) textureGather(tex, pos)
    #else
        #define samplerStereo sampler2DArray
#if !defined(SHADER_GL410)
        #define GS_LAYER_COPY gl_Layer = eye_gs[0]
        #define FS_LAYER_TO_GLOBAL
        #define texStereo(tex, pos) texture(tex, vec3(vec2(pos).x, vec2(pos).y, gl_Layer))
        #define texStereolod(tex, pos) textureLod(tex, vec3((pos).x, (pos).y, gl_Layer), (pos).w)
        #define texStereoNoLod(tex, pos) textureLod(tex, vec3((pos).x, (pos).y, gl_Layer), 0.)
        #define texStereoOffset(tex, pos, offset) textureOffset(tex, vec3((pos).x, (pos).y, gl_Layer), offset)
        #define texStereoOffsetNoLod(tex, pos, offset) textureLodOffset(tex, vec3((pos).x, (pos).y, gl_Layer), 0.0, offset)
        #define texStereoGather(tex, pos) textureGather(tex, vec3((pos).x, (pos).y, gl_Layer))
#else
        #define GS_LAYER_COPY gl_Layer = eye_gs[0]; glLayer_gs = gl_Layer
        #define FS_LAYER_TO_GLOBAL g_glLayer = glLayer_gs
        #define texStereo(tex, pos) texture(tex, vec3(vec2(pos).x, vec2(pos).y, g_glLayer))
        #define texStereolod(tex, pos) textureLod(tex, vec3((pos).x, (pos).y, g_glLayer), (pos).w)
        #define texStereoNoLod(tex, pos) textureLod(tex, vec3((pos).x, (pos).y, g_glLayer), 0.)
        #define texStereoOffset(tex, pos, offset) textureOffset(tex, vec3((pos).x, (pos).y, g_glLayer), offset)
        #define texStereoOffsetNoLod(tex, pos, offset) textureLodOffset(tex, vec3((pos).x, (pos).y, g_glLayer), 0.0, offset)
        #define texStereoGather(tex, pos) textureGather(tex, vec3((pos).x, (pos).y, g_glLayer))
#endif
    #endif

    #if USE_GEOMETRY_SHADER
        #define VS_OUT(typ, name) out typ name##_gs
        #define VS_OUT_NOPERSP_NOGEOM(typ, name)
        #define VS_OUT_EYE out int eye_gs
        #define VS_VARYING(var, val) var##_gs = val
        #define VS_VARYING_NOGEOM(var, val)
        #define VS_POSITION(val_vs, val_gs) gl_Position = val_gs
        #define VS_EYE int eye_vs = gl_InstanceID + layer; eye_gs = eye_vs
    #else
        #define VS_OUT(typ, name) out typ name
        #define VS_OUT_NOPERSP_NOGEOM(typ, name) noperspective out typ name
        #define VS_OUT_EYE flat out float eye
        #define VS_VARYING(var, val) var = val
        #define VS_VARYING_NOGEOM(var, val) var = val
        #define VS_POSITION(val_vs, val_gs) gl_Position = val_vs
        #define VS_EYE int eye_vs = gl_InstanceID + layer; eye = float(eye_vs)
    #endif

//**************************************************************************
// HLSL defines to support common shader code
#else
    #define HLSL
    // disable warning 'pow(f, e) will not work for negative f, use abs(f) or conditionally handle negative values if you expect them' as it is fairly common and not actionable.
    #pragma warning(disable : 3571)

    #define N_EYES 1
    #define FLT_MIN_VALUE 0.0000001
    #define texNoLod(tex, pos) tex2Dlod(tex, float4(pos, 0., 0.))
    #define texStereoNoLod(tex, pos) texNoLod(tex, pos)
    #define BRANCH [branch]
    #define UNROLL [unroll]
    #define UNIFORM const
    #define to_float3x3 (float3x3)
    #define MAT3_BEGIN {
    #define MAT_END }
    #define MAT_ROW3_BEGIN {
    #define MAT_ROW_END }
#endif


//**************************************************************************
// Common definitions for OpenGL & HLSL

#ifdef SHADER_GL410
int g_glLayer = 0;
#endif

#define PI 3.1415926535897932384626433832795

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

#if 0
float acos_approx(const float v)
{
    const float x = abs(v);
    if(1. - x <= FLT_MIN_VALUE) // necessary due to compiler doing 1./rsqrt instead of sqrt
       return (v >= 0.) ? 0. : PI;
    const float res = (-0.155972/*C1*/ * x + 1.56467/*C0*/) * sqrt(1. - x);
    return (v >= 0.) ? res : PI - res;
}
#endif

float acos_approx_divPI(const float v)
{
    //return acos(v)*(1./PI);

    const float x = abs(v);
    if(1. - x <= FLT_MIN_VALUE) // necessary due to compiler doing 1./rsqrt instead of sqrt
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
    if(1. - x <= FLT_MIN_VALUE) // necessary due to compiler doing 1./rsqrt instead of sqrt
       return (v >= 0.) ? (0.5*PI) : -(0.5*PI);
    const float res = (-0.155972/*C1*/ * x + 1.56467/*C0*/) * sqrt(1. - x);
    return (v >= 0.) ? (0.5*PI) - res : -(0.5*PI) + res;
}
#endif

float asin_approx_divPI(const float v)
{
	//return asin(v) / PI;

    const float x = abs(v);
    if(1. - x <= FLT_MIN_VALUE) // necessary due to compiler doing 1./rsqrt instead of sqrt
       return (v >= 0.) ? 0.5 : -0.5;
    const float res = ((-0.155972/PI)/*C1*/ * x + (1.56467/PI)/*C0*/) * sqrt(1. - x);
    return (v >= 0.) ? 0.5 - res : -0.5 + res;
}

float2 ray_to_equirectangular_uv(const float3 ray)
{
   return float2( // remap to 2D envmap coords
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

float4x4 inverse4x4(const float4x4 m)
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

   float4x4 ret;

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

float3 InvReinhardToneMap(const float3 color)
{
    const float inv_2bh = 0.5 / BURN_HIGHLIGHTS;
    const float bh = 4.0 * BURN_HIGHLIGHTS - 2.0;
    return (color - 1.0 + sqrt(color * (color + bh) + 1.0)) * inv_2bh;
}

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

float3 InvGamma(const float3 color) //!! use hardware support? D3DSAMP_SRGBTEXTURE
{
    return float3(InvGamma(color.x),InvGamma(color.y),InvGamma(color.z));
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
float2 FBGamma(const float2 color)
{
    return float2(FBGamma(color.x),FBGamma(color.y));
}
float3 FBGamma(const float3 color)
{
    return float3(FBGamma(color.x),FBGamma(color.y),FBGamma(color.z));
}
#else
#define FBGamma // uses hardware support via D3DRS_SRGBWRITEENABLE
#endif

#define Luminance(linearRGB) dot(linearRGB, float3(0.212655,0.715158,0.072187))



#if 0
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
	cNew = lerp(cBase*cBlend*2.0, 1.0-2.0*(1.0-cBase)*(1.0-cBlend), cNew);

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
	cNew = max(lerp(cBase*cBlend*2.0, 1.0-2.0*(1.0-cBase)*(1.0-cBlend), cNew), float4(0.,0.,0.,0.));

	//cNew.a = 1.0;
	return cNew;
}
