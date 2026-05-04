// license:GPLv3+

$input a_position, a_texcoord0
$output v_texcoord0, v_texcoord1
#ifdef CLIP
	$output v_clipDistance
#endif

#include "common.sh"

#ifdef WORLD
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
#endif

uniform vec4 glassArea;
uniform vec4 glassPad;
#define glassPadLeft (glassPad.x)
#define glassPadRight (glassPad.y)
#define glassPadTop (glassPad.z)
#define glassPadBottom (glassPad.w)

void main()
{
	vec4 pos = vec4(a_position, 1.0);
	v_texcoord0 = glassArea.xy + a_texcoord0 * glassArea.zw; // Glass
	v_texcoord1 = a_texcoord0 * vec2(1.0 + glassPadLeft + glassPadRight, 1.0 + glassPadTop + glassPadBottom) - vec2(glassPadLeft, glassPadTop); // Display
	#ifdef STEREO
		gl_Layer = gl_InstanceID;
	#endif
	#ifdef WORLD
		// Camera-relative clip transform: subtract camera world pos on the GPU to keep magnitudes small.
		vec3 wpos_rel = a_position.xyz - cCameraPosWorld;
		gl_Position = mul(mRotViewProj, vec4(wpos_rel, 1.0));
		#ifdef CLIP
			v_clipDistance = dot(clip_plane, pos);
		#endif
	#else
		// Set Z to 1. which in turns result in a written depth of 0. needed to avoid tonemapping of DMD and for correct fake stereo
		// (background and DMD are placed at depth buffer = 0.0, that is to say behind the playfield, at max separation)
		gl_Position = vec4(a_position.x, a_position.y, 1.0, 1.0);
	#endif
}
