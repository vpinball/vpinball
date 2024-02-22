$input a_position
$input a_texcoord0
$output v_texcoord0

#include "bgfx_shader.sh"

// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

void main()
{
	gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);

// #if BGFX_SHADER_LNGUAGE_HLSL == s_3_0
   // DirectX 9 has 0,0 at the top left corner of the first texel. Pixel perfect sampling needs coordinates to be offseted by half a texel.
//   v_texcoord0 = a_texcoord0 + 0.5 * w_h_height.xy;
//#else
   v_texcoord0 = a_texcoord0;
//#endif
}
