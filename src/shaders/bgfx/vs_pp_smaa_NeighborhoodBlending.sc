$input a_position, a_texcoord0
#ifdef STEREO
$output v_texcoord0, v_texcoord2, v_eye
uniform vec4 layer;
#else
$output v_texcoord0, v_texcoord2
#endif

#include "common.sh"

#define SMAA_INCLUDE_VS 1
#define SMAA_INCLUDE_PS 0
#include "SMAA.sh"

void main()
{
	vec4 offset;
	SMAANeighborhoodBlendingVS(a_texcoord0.xy, offset);
	v_texcoord0 = a_texcoord0.xy;
	v_texcoord2 = offset;
    gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
    #ifdef STEREO
       gl_Layer = gl_InstanceID;
       v_eye = layer.x + gl_InstanceID;
    #endif
}
