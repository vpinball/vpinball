// license:GPLv3+

$input a_position
$output v_tablePos
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
	#ifdef STEREO
		gl_Layer = gl_InstanceID;
	#endif
	#ifdef CLIP
		v_clipDistance = dot(clip_plane, pos);
	#endif
	// Camera-relative clip transform: subtract camera world pos on the GPU to keep magnitudes small.
	vec3 wpos_rel = a_position.xyz - cCameraPosWorld;
	gl_Position = mul(mRotViewProj, vec4(wpos_rel, 1.0));
	//Disabled since this would move backdrop lights behind other backdrop parts (rendered at z = 0), this could be readded with a max at 0, but I don't get why we would need this type of hack
	//gl_Position.z = max(gl_Position.z, 0.00006103515625); // clamp lights to near clip plane to avoid them being partially clipped // 0.00006103515625 due to 16bit half float
}
