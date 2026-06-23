// license:GPLv3+

$input v_texcoord0, v_eye

#include "common.sh"


SAMPLER2DSTEREO(tex_fb_unfiltered, 0); // Framebuffer (unfiltered)
SAMPLER2DSTEREO(tex_depth,         4); // DepthBuffer

#define threshold     1.5/255.0
#define replacement   threshold

void main()
{
	if (texStereoNoLod(tex_depth, v_texcoord0).x == 1.0)
	{
		// Fixed color keyed passthrough
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{
		// Solid part, ensure we won't be filtered by the fixed color key
		vec4 color = texStereo(tex_fb_unfiltered, v_texcoord0);
		bool isPassthrough = all(lessThan(color.rgb, vec3_splat(threshold)));
		gl_FragColor = isPassthrough ? vec4(replacement, replacement, replacement, 1.0) : color;
	}
}
