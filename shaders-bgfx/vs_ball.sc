$input a_position, a_normal, a_texcoord0
$output v_worldPos, v_normal, v_texcoord0

#include "bgfx_shader.sh"

uniform mat4 orientation;

void main()
{
	// apply spinning and move the ball to it's actual position
	vec4 vPosition4 = vec4(a_position, 1.0);
	vPosition4.xyz = mul(orientation, vPosition4).xyz;

	// apply spinning to the normals too to get the sphere mapping effect
	const vec3 nspin = mul(orientation, vec4(a_normal, 0.0)).xyz;
	v_normal = normalize(mul(u_modelView, vec4(nspin, 0.0)).xyz); // actually: mul(vec4(nspin,0.), matWorldViewInverseTranspose), but optimized to save one matrix

	v_texcoord0 = a_texcoord0;
	v_worldPos = mul(u_modelView, vPosition4).xyz;
	gl_Position = mul(u_modelViewProj, vPosition4);
}
