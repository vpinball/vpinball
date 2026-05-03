// license:GPLv3+

$input a_position, a_normal, a_texcoord0
$output v_worldPos, v_normal, v_texcoord0
#ifdef STEREO
    $output v_eye
#endif
#ifdef CLIP
    $output v_clipDistance
#endif

#include "common.sh"

uniform mat4 orientation;
#ifdef STEREO
    uniform vec4 layer;
    uniform mat4 matWorldView[2];
    uniform mat4 matWorldViewInverse[2];
    uniform mat4 matWorldViewProj[2];
    #define mWorldView        matWorldView[gl_InstanceID]
    #define mWorldViewInverse matWorldViewInverse[gl_InstanceID]
    #define mWorldViewProj    matWorldViewProj[gl_InstanceID]
#else
    uniform mat4 matWorldView;
    uniform mat4 matWorldViewInverse;
    uniform mat4 matWorldViewProj;
    #define mWorldView        matWorldView
    #define mWorldViewInverse matWorldViewInverse
    #define mWorldViewProj    matWorldViewProj
#endif
#ifdef CLIP
    uniform vec4 clip_plane;
#endif

void main()
{
    // apply spinning and move the ball to it's actual position
    vec4 pos = vec4(a_position, 1.0);
    vec4 tpos = mul(orientation, pos);
    pos.xyz = tpos.xyz;
    #ifdef CLIP
        v_clipDistance = dot(clip_plane, tpos);
    #endif

    // apply spinning to the normals too to get the sphere mapping effect
    const vec3 nspin = mul(orientation, vec4(a_normal, 0.0)).xyz;
    // Needs to use a 'normal' matrix, and to normalize since we allow non uniform stretching, therefore mWorldView is not orthonormal
    v_normal = normalize(mul(vec4(nspin, 0.0), mWorldViewInverse).xyz); // actually: mul(vec4(nspin,0.), mWorldViewInverseTranspose), but optimized to save one matrix

    v_texcoord0 = a_texcoord0;
    v_worldPos = mul(mWorldView, pos).xyz;
    #ifdef STEREO
        gl_Layer = gl_InstanceID;
        v_eye = layer.x + gl_InstanceID;
    #endif
    gl_Position = mul(mWorldViewProj, pos);
}
