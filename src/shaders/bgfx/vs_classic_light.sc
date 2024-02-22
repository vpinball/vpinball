$input a_position, a_normal, a_texcoord0
$output v_worldPos, v_tablePos, v_normal, v_texcoord0

#include "bgfx_shader.sh"

uniform mat4 matWorldView;
uniform mat4 matWorldViewInverseTranspose;
uniform mat4 matView;


void main()
{
	vec4 vPosition4 = vec4(a_position, 1.0);
	v_tablePos = a_position.xyz;
	v_worldPos = mul(matWorldView, vPosition4).xyz;
	// Due to layback, we use non affine transform so we need to use inverse transpose of the world view matrix
	v_normal = normalize(mul(matWorldViewInverseTranspose, vec4(a_normal, 0.0)).xyz);
#ifdef TEX
	v_texcoord0 = a_texcoord0;
#endif
	gl_Position = mul(u_modelViewProj, vPosition4);
	gl_Position.z = max(gl_Position.z, 0.00001); // clamp lights to near clip plane to avoid them being partially clipped
}
