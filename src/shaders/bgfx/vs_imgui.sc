$input a_position, a_normal, a_texcoord0
$output v_color0, v_texcoord0

#include "common.sh"

uniform mat4 matWorldView;

uniform vec4 staticColor_Alpha;
#define stereoOfs staticColor_Alpha.x
#define sdrScale staticColor_Alpha.a

void main()
{
    vec4 ofsPos = vec4(a_position.xy, 0.0, 1.0);
	#ifdef STEREO
		if (gl_InstanceID == 0)
			ofsPos.x += stereoOfs;
		else
			ofsPos.x -= stereoOfs;
		gl_Layer = gl_InstanceID;
	#endif
	vec4 pos = mul(matWorldView, ofsPos);
	gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
	
	v_texcoord0 = a_texcoord0;

	v_color0 = sdrScale * vec4(a_normal.rgb, a_position.z);
}