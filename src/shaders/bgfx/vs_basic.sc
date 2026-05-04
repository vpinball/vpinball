// license:GPLv3+

$input a_position, a_normal, a_texcoord0
$output v_worldPos, v_tablePos, v_normal, v_texcoord0
#ifdef STEREO
    $output v_eye
#endif
#ifdef CLIP
    $output v_clipDistance
#endif

#include "common.sh"

uniform mat4 matWorld;
#ifdef STEREO
    uniform vec4 layer;
    uniform mat4 matView[2];
    uniform mat4 matWorldView[2];
    uniform mat4 matWorldViewInverseTranspose[2];
    uniform mat4 matRotViewProj[2];
    uniform vec4 cameraPosWorld[2];
    #define mView                      matView[int(round(v_eye))]
    #define mWorldView                 matWorldView[int(round(v_eye))]
    #define mWorldViewInverseTranspose matWorldViewInverseTranspose[int(round(v_eye))]
    #define mRotViewProj               matRotViewProj[int(round(v_eye))]
    #define cCameraPosWorld            cameraPosWorld[int(round(v_eye))].xyz
#else
    uniform mat4 matView;
    uniform mat4 matWorldView;
    uniform mat4 matWorldViewInverseTranspose;
    uniform mat4 matRotViewProj;
    uniform vec4 cameraPosWorld;
    #define mView                      matView
    #define mWorldView                 matWorldView
    #define mWorldViewInverseTranspose matWorldViewInverseTranspose
    #define mRotViewProj               matRotViewProj
    #define cCameraPosWorld            cameraPosWorld.xyz
#endif
#ifdef CLIP
    uniform vec4 clip_plane;
#endif

void main()
{
    #ifdef STEREO
        gl_Layer = gl_InstanceID;
        v_eye = layer.x + gl_InstanceID;
    #endif
    vec4 pos = vec4(a_position, 1.0);
    vec4 tpos = mul(matWorld, pos);
    v_tablePos = tpos.xyz;
    v_worldPos = mul(mWorldView, pos).xyz;
    #ifdef CLIP
        v_clipDistance = dot(clip_plane, tpos);
    #endif
    // Due to layback, we use non affine transform so we need to use inverse transpose of the world view matrix
    v_normal = normalize(mul(mWorldViewInverseTranspose, vec4(a_normal, 0.0)).xyz);
    //v_normal = normalize(mul(mWorldView, vec4(a_normal, 0.0)).xyz);
    // Camera-relative clip-space transform: subtract the camera's world position from the world-space
    // vertex position and apply (viewRotation x proj). This avoids GPU f32 precision loss on Adreno (Quest)
    // that comes from baking the large camera translation into matWorldViewProj on the CPU.
    vec3 wpos_rel = tpos.xyz - cCameraPosWorld;
    gl_Position = mul(mRotViewProj, vec4(wpos_rel, 1.0));
    #ifdef KICKER
        vec4 tpos_kick = mul(matWorld, vec4(pos.x, pos.y, pos.z - 30.0, 1.0));
        vec3 wpos_rel_kick = tpos_kick.xyz - cCameraPosWorld;
        gl_Position.z = mul(mRotViewProj, vec4(wpos_rel_kick, 1.0)).z;
    #elif defined(CLASSIC_LIGHT)
        gl_Position.z = max(gl_Position.z, 0.00006103515625); // clamp lights to near clip plane to avoid them being partially clipped // 0.00006103515625 due to 16bit half float
    #endif
    #ifdef TEX
        v_texcoord0 = a_texcoord0;
    #endif
}
