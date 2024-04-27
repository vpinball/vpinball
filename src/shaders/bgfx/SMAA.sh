uniform vec4 u_SmaaMetrics;

#define SMAA_PRESET_MEDIUM
//#define SMAA_PRESET_HIGH
//#define SMAA_USE_DEPTH
//#define SMAA_USE_STENCIL
//#define SMAA_USE_COLOR // otherwise luma

uniform vec4 w_h_height; 
#define SMAA_RT_METRICS w_h_height

#if BGFX_SHADER_LANGUAGE_GLSL
#define SMAA_GLSL_3
#else
#define SMAA_HLSL_4
#endif

#include "SMAA.hlsl"
