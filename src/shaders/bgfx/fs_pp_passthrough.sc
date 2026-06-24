// license:GPLv3+

$input v_texcoord0, v_eye

#include "common.sh"


SAMPLER2DSTEREO(tex_fb_unfiltered, 0); // Framebuffer (unfiltered)
SAMPLER2DSTEREO(tex_depth,         4); // DepthBuffer

#define threshold     1.0/255.0

void main()
{
	if (texStereoNoLod(tex_depth, v_texcoord0).x == 1.0)
	{
		// Fixed color keyed passthrough
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else
	{
		// Solid part, compress color to ensure we won't be filtered by the fixed color key
		vec3 color = texStereoNoLod(tex_fb_unfiltered, v_texcoord0).rgb;
		color = vec3_splat(threshold) + (1.0 - threshold) * color;
		gl_FragColor = vec4(color, 1.0);
	}
}
