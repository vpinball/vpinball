$input a_position, a_texcoord0, a_color0
$output v_color0, v_texcoord0

#include "common.sh"

uniform vec4 u_sdrScale;
uniform vec4 u_stereoOfs;

void main()
{
    vec4 ofsPos = vec4(a_position.xy, 0.0, 1.0);
	#ifdef STEREO
		if (gl_InstanceID == 0)
			ofsPos.x += u_stereoOfs.x;
		else
			ofsPos.x -= u_stereoOfs.x;
		gl_Layer = gl_InstanceID;
	#endif
	vec4 pos = mul(u_viewProj, ofsPos);
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
	
	v_texcoord0 = a_texcoord0;

	v_color0 = a_color0 * u_sdrScale;
}