// license:GPLv3+

$input a_position, a_texcoord0
$output v_tablePos, v_texcoord0
#ifdef CLIP
	$output v_clipDistance
#endif

#include "common.sh"

#ifdef STEREO
	uniform mat4 matWorldViewProj[2];
	#define mvp matWorldViewProj[gl_InstanceID]
#else
	uniform mat4 matWorldViewProj;
	#define mvp matWorldViewProj
#endif
#ifdef CLIP
	uniform vec4 clip_plane;
#endif

void main()
{
	vec4 pos = vec4(a_position, 1.0);
	v_tablePos = a_position.xyz;
	v_texcoord0 = a_texcoord0;
	#ifdef STEREO
		gl_Layer = gl_InstanceID;
	#endif
	#ifdef CLIP
		v_clipDistance = dot(clip_plane, pos);
	#endif
	gl_Position = mul(mvp, pos);
}
