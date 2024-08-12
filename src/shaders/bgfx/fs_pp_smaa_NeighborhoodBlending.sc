// license:GPLv3+

$input v_texcoord0, v_texcoord2

#include "common.sh"

#define SMAA_INCLUDE_VS 0
#define SMAA_INCLUDE_PS 1
#include "SMAA.sh"

SAMPLER2D(tex_fb_filtered, 0);
SAMPLER2D(blendTex,        8);

void main()
{
#if BGFX_SHADER_LANGUAGE_GLSL
	gl_FragColor = SMAANeighborhoodBlendingPS(v_texcoord0, v_texcoord2, tex_fb_filtered, blendTex);
#else
	gl_FragColor = SMAANeighborhoodBlendingPS(v_texcoord0, v_texcoord2, tex_fb_filtered.m_texture, blendTex.m_texture);
#endif
}
