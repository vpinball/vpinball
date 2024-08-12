// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"

SAMPLER2DSTEREO(tex_fb_filtered,  0); // Framebuffer (filtered)

void main()
{
	gl_FragColor = vec4(texStereoNoLod(tex_fb_filtered, v_texcoord0).rgb, 1.0);
}
