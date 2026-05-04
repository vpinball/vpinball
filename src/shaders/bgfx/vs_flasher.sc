// license:GPLv3+

$input a_position, a_texcoord0
$output v_tablePos, v_texcoord0
#ifdef CLIP
	$output v_clipDistance
#endif

#include "common.sh"

#ifdef STEREO
	uniform mat4 matRotViewProj[2];
	uniform vec4 cameraPosWorld[2];
	#define mRotViewProj    matRotViewProj[gl_InstanceID]
	#define cCameraPosWorld cameraPosWorld[gl_InstanceID].xyz
#else
	uniform mat4 matRotViewProj;
	uniform vec4 cameraPosWorld;
	#define mRotViewProj    matRotViewProj
	#define cCameraPosWorld cameraPosWorld.xyz
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
	// Camera-relative clip transform: subtract camera world pos on the GPU to keep magnitudes small.
	vec3 wpos_rel = a_position.xyz - cCameraPosWorld;
	gl_Position = mul(mRotViewProj, vec4(wpos_rel, 1.0));
}
