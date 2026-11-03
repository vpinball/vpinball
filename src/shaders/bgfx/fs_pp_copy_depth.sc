// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"

SAMPLER2DSTEREO(tex_depth, 4); // Depth buffer

void main()
{
	gl_FragDepth = texStereoNoLod(tex_depth, v_texcoord0).x;
	gl_FragColor = vec4_splat(0.0);
}
