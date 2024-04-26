$input a_position, a_texcoord0
$output v_tablePos, v_texcoord0

#include "common.sh"

#ifdef STEREO
	uniform mat4 matWorldViewProj[2];
#else
	uniform mat4 matWorldViewProj;
#endif

void main()
{
    v_tablePos = a_position.xyz;
    v_texcoord0 = a_texcoord0;
	#ifdef STEREO
		gl_Position = mul(matWorldViewProj[gl_InstanceID], vec4(a_position, 1.0));
		gl_Layer = gl_InstanceID;
	#else
		gl_Position = mul(matWorldViewProj, vec4(a_position, 1.0));
	#endif
}
