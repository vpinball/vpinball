////GLOBAL
//HLSL to GLSL helpers
#define clip(x) {if (x<0) {discard;}}
#define ddx(x) dFdx(x)
#define ddy(x) dFdy(x)
#define lerp(x,y,s) mix(x,y,s)
#define saturate(x) clamp(x,0.0,1.0)
#define any(x) (dot(x,x)>0.0005)
#define sincos(phi,sp,cp) {sp=sin(phi);cp=cos(phi);}
#define frac(x) fract(x)
#define rsqrt(x) inversesqrt(x)
#define rcp(x) (1.0/(x))

#define tex2Dlod(sampler, v) textureLod(sampler, (v).xy,(v).w)
#define tex2D(sampler, v) texture(sampler, v)

#define const

#define float4 vec4
#define float3 vec3
#define float2 vec2

#define float4x4 mat4
#define float3x3 mat3
#define float2x2 mat2

#define float4x3 mat4x3
#define float3x4 mat3x4

//Not working :-(
//vec4 pow(vec4 b, float e) {return pow(b,vec4(e));}
//vec3 pow(vec3 b, float e) {return pow(b,vec3(e));}
//vec2 pow(vec2 b, float e) {return pow(b,vec2(e));}
