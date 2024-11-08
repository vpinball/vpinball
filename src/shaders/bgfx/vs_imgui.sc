$input a_position, a_texcoord0, a_color0
$output v_color0, v_texcoord0

#include "common.sh"

uniform vec4 staticColor_Alpha;

void main()
{
	vec4 pos = mul(u_viewProj, vec4(a_position.xy, 0.0, 1.0) );
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
	v_texcoord0 = a_texcoord0;
	v_color0    = a_color0 * staticColor_Alpha;
}