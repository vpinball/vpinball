$input a_position, a_texcoord0
$output v_texcoord0

#include "common.sh"

#ifdef STEREO
	uniform mat4 matWorldViewProj[2];
#else
	uniform mat4 matWorldViewProj;
#endif

void main()
{
    v_texcoord0 = a_texcoord0;
#ifdef WORLD
	#ifdef STEREO
		gl_Position = mul(matWorldViewProj[gl_InstanceID], vec4(a_position, 1.0));
		gl_Layer = gl_InstanceID;
	#else
		gl_Position = mul(matWorldViewProj, vec4(a_position, 1.0));
	#endif
#else
    // Set Z to 1. which in turns result in a written depth of 0. needed to avoid tonemapping of DMD and for correct fake stereo
    // (background and DMD are placed at depth buffer = 0.0, that is to say behind the playfield, at max separation)
    gl_Position = vec4(a_position.x, a_position.y, 1.0, 1.0);
#endif
}
