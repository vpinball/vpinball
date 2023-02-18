$input v_texcoord0

#include "bgfx_shader.sh"


// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

SAMPLER2D(tex_fb_unfiltered,  0); // Framebuffer (unfiltered)

void main()
{
	gl_FragColor = vec4(texture2DLod(tex_fb_unfiltered, v_texcoord0, 0.0).rgb * (1. / w_h_height.z), 1.0);
}
