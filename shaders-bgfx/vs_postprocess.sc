$input a_position
$input a_texcoord0
$output v_texcoord0

#include "bgfx_shader.sh"

void main()
{
	gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);

#if BGFX_SHADER_LNGUAGE_HLSL == s_3_0
   // DirectX 9 has 0,0 at the top left corner of the first texel. Pixel perfect sampling needs coordinates to be offseted by half a texel.
   v_texcoord0 = a_texcoord0 + 0.5 * u_viewTexel.xy;
#else
   v_texcoord0 = a_texcoord0;
#endif
}
