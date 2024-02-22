$input a_position, a_texcoord0
$output v_texcoord0

#include "bgfx_shader.sh"

uniform mat4 matWorldViewProj;

void main()
{
	v_texcoord0 = a_texcoord0;
#ifdef WORLD
	gl_Position = mul(matWorldViewProj, vec4(a_position, 1.0));
#else
	gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
#endif
}
