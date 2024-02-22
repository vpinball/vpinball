$input a_position, a_texcoord0
$output v_tablePos, v_texcoord0

#include "bgfx_shader.sh"

uniform mat4 matWorldViewProj;

void main()
{
	v_tablePos = a_position.xyz;
	v_texcoord0 = a_texcoord0;
	gl_Position = mul(matWorldViewProj, vec4(a_position, 1.0) );
}
