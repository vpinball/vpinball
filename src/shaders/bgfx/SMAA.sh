uniform vec4 u_SmaaMetrics;

//#define SMAA_PRESET_MEDIUM
#define SMAA_PRESET_HIGH
//#define SMAA_USE_DEPTH
//#define SMAA_USE_STENCIL
#define SMAA_USE_COLOR // otherwise luma

uniform vec4 w_h_height; 
#define SMAA_RT_METRICS w_h_height

#if BGFX_SHADER_LANGUAGE_GLSL
#define SMAA_GLSL_3
#elif BGFX_SHADER_LANGUAGE_SPIRV
// Vulkan: the HLSL path declares standalone SamplerState (Linear/PointSampler) that bgfx does not
// add to the pipeline layout, so RADV crashes compiling the pipeline and the AA renders broken.
// Use a custom porting that samples through bgfx combined samplers (like the working OpenGL build),
// while keeping bgfx's HLSL-style types (the GLSL path would redefine bvec2/float2 and clash).
#define SMAA_CUSTOM_SL
#define SMAATexture2D(tex) sampler2D tex
#define SMAATexturePass2D(tex) tex
#define SMAASampleLevelZero(tex, coord) texture2DLod(tex, coord, 0.0)
#define SMAASampleLevelZeroPoint(tex, coord) texture2DLod(tex, coord, 0.0)
#define SMAASampleLevelZeroOffset(tex, coord, offset) texture2DLodOffset(tex, coord, 0.0, offset)
#define SMAASample(tex, coord) texture2D(tex, coord)
#define SMAASamplePoint(tex, coord) texture2D(tex, coord)
#define SMAASampleOffset(tex, coord, offset) texture2DLodOffset(tex, coord, 0.0, offset)
#define SMAA_FLATTEN
#define SMAA_BRANCH
#else
#define SMAA_HLSL_4_1
#endif

#include "SMAA.hlsl"
