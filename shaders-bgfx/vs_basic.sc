$input a_position, a_normal, a_texcoord0
$output v_worldPos, v_tablePos, v_normal, v_texcoord0

#include "bgfx_shader.sh"

void main()
{
	vec4 vPosition4 = vec4(a_position, 1.0);
	v_tablePos = mul(u_model[0], vPosition4).xyz;
	v_worldPos = mul(u_modelView, vPosition4).xyz;
	v_normal = normalize(mul(u_modelView, vec4(a_normal, 0.0)).xyz);
	//v_normal = normalize((matWorldViewInverseTranspose*a_normal).xyz);
#ifdef TEX
	v_texcoord0 = a_texcoord0;
#endif
	gl_Position = mul(u_modelViewProj, vPosition4);
}
