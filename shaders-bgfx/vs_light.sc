$input a_position
$output v_tablePos

#include "bgfx_shader.sh"

void main()
{
	v_tablePos = a_position.xyz;
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0) );
	gl_Position.z = max(gl_Position.z, 0.00001); // clamp lights to near clip plane to avoid them being partially clipped
}
