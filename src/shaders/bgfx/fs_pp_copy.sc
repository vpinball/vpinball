$input v_texcoord0

#include "common.sh"

SAMPLER2D(tex_fb_filtered,  0); // Framebuffer (filtered)

void main()
{
	gl_FragColor = vec4(texture2DLod(tex_fb_filtered, v_texcoord0, 0.0).rgb, 1.0);
}
