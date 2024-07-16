$input a_position, a_texcoord0
$output v_texcoord0
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
    v_texcoord0 = a_texcoord0;
	#ifdef WORLD
		gl_Position = mul(mvp, pos);
		#ifdef STEREO
			gl_Layer = gl_InstanceID;
		#endif
		#ifdef CLIP
			v_clipDistance = dot(clip_plane, pos);
		#endif
	#else
		// Set Z to 1. which in turns result in a written depth of 0. needed to avoid tonemapping of DMD and for correct fake stereo
		// (background and DMD are placed at depth buffer = 0.0, that is to say behind the playfield, at max separation)
		gl_Position = vec4(a_position.x, a_position.y, 1.0, 1.0);
	#endif
}
