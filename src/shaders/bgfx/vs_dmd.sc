// license:GPLv3+

$input a_position, a_texcoord0
$output v_texcoord0, v_texcoord1
#ifdef CLIP
	$output v_clipDistance
#endif

#include "common.sh"

#ifdef WORLD
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
		gl_Position = mul(mvp, pos);
		#ifdef CLIP
			v_clipDistance = dot(clip_plane, pos);
		#endif
	#else
		// Set Z to 1. which in turns result in a written depth of 0. needed to avoid tonemapping of DMD and for correct fake stereo
		// (background and DMD are placed at depth buffer = 0.0, that is to say behind the playfield, at max separation)
		gl_Position = vec4(a_position.x, a_position.y, 1.0, 1.0);
	#endif
}
