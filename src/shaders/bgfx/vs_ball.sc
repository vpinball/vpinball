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
    uniform mat4 matRotViewProj[2];
    uniform vec4 cameraPosWorld[2];
    #define mWorldView        matWorldView[gl_InstanceID]
    #define mWorldViewInverse matWorldViewInverse[gl_InstanceID]
    #define mRotViewProj      matRotViewProj[gl_InstanceID]
    #define cCameraPosWorld   cameraPosWorld[gl_InstanceID].xyz
#else
    uniform mat4 matWorldView;
    uniform mat4 matWorldViewInverse;
    uniform mat4 matRotViewProj;
    uniform vec4 cameraPosWorld;
    #define mWorldView        matWorldView
    #define mWorldViewInverse matWorldViewInverse
    #define mRotViewProj      matRotViewProj
    #define cCameraPosWorld   cameraPosWorld.xyz
#endif
#ifdef CLIP
    uniform vec4 clip_plane;
#endif

void main()
{
    // apply spinning and move the ball to it's actual position (orientation contains rot * scale * translate)
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
    // Camera-relative clip transform: subtract camera world pos on the GPU to keep magnitudes small.
    vec3 wpos_rel = tpos.xyz - cCameraPosWorld;
    gl_Position = mul(mRotViewProj, vec4(wpos_rel, 1.0));
}
