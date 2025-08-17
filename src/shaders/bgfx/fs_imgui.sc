$input v_color0, v_texcoord0

#include "common.sh"

SAMPLER2D      (tex_base_color, 0); // base color

void main()
{
	gl_FragColor = texture2D(tex_base_color, v_texcoord0) * v_color0;
}
