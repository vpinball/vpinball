$input a_position, a_texcoord0
$output v_texcoord0, v_texcoord2

#include "common.sh"

#define SMAA_INCLUDE_VS 1
#define SMAA_INCLUDE_PS 0
#include "SMAA.sh"

#ifdef STEREO
	uniform mat4 matWorldViewProj[2];
	#define mvp matWorldViewProj[gl_InstanceID]
#else
	uniform mat4 matWorldViewProj;
	#define mvp matWorldViewProj
#endif
void main()
{
	vec4 offset;
	SMAANeighborhoodBlendingVS(a_texcoord0.xy, offset);
	v_texcoord0 = a_texcoord0.xy;
	v_texcoord2 = offset;
	gl_Position = mul(mvp, vec4(a_position, 1.0));
}
