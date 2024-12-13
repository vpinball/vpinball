// license:GPLv3+

$input a_position, a_normal, a_texcoord0
$output v_worldPos, v_tablePos, v_normal, v_texcoord0

#include "common.sh"

uniform mat4 matWorldViewProj[2];

void main()
{
    int eye = int(a_normal.x);
	gl_Position = mul(matWorldViewProj[eye], vec4(a_position, 1.0));
	gl_Position.z = 0.0;
	gl_Layer = eye;
}
