// license:GPLv3+

$input a_position, a_normal, a_texcoord0
$output v_worldPos
#ifdef STEREO
	$output v_eye
#endif
#ifdef CLIP
	$output v_clipDistance
#endif

#include "common.sh"

uniform mat4 matWorldView;
uniform mat4 matWorldViewInverse;
#ifdef STEREO
	uniform vec4 layer;
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
	v_worldPos = vec3(a_texcoord0.x, a_texcoord0.y, a_normal.x); // not exactly worlpos...
	#ifdef CLIP
		v_clipDistance = dot(clip_plane, pos);
	#endif
	#ifdef STEREO
		gl_Layer = gl_InstanceID;
		v_eye = layer.x + gl_InstanceID;
	#endif
	gl_Position = mul(mvp, pos);
}
