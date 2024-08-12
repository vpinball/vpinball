// license:GPLv3+

$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4

#include "common.sh"

#define SMAA_INCLUDE_VS 0
#define SMAA_INCLUDE_PS 1
#include "SMAA.sh"

SAMPLER2D(edgesTex,   7);
SAMPLER2D(areaTex,    9);
SAMPLER2D(searchTex, 10);

void main()
{
	vec4 offset[3];
	offset[0] = v_texcoord2;
	offset[1] = v_texcoord3;
	offset[2] = v_texcoord4;
	vec4 subsampleIndices = vec4_splat(0.0);
#if BGFX_SHADER_LANGUAGE_GLSL
	gl_FragColor = SMAABlendingWeightCalculationPS(v_texcoord0, v_texcoord1, offset, edgesTex, areaTex, searchTex, subsampleIndices);
#else
	gl_FragColor = SMAABlendingWeightCalculationPS(v_texcoord0, v_texcoord1, offset, edgesTex.m_texture, areaTex.m_texture, searchTex.m_texture, subsampleIndices);
#endif
}
