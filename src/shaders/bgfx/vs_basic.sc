$input a_position, a_normal, a_texcoord0
$output v_worldPos, v_tablePos, v_normal, v_texcoord0

#include "common.sh"

uniform mat4 matWorld;
uniform mat4 matView;
uniform mat4 matWorldView;
uniform mat4 matWorldViewInverseTranspose;
uniform mat4 matWorldViewProj;

void main()
{
    vec4 pos = vec4(a_position, 1.0);
    v_tablePos = mul(matWorld, pos).xyz;
    v_worldPos = mul(matWorldView, pos).xyz;
    // Due to layback, we use non affine transform so we need to use inverse transpose of the world view matrix
    v_normal = normalize(mul(matWorldViewInverseTranspose, vec4(a_normal, 0.0)).xyz);
    //v_normal = normalize(mul(matWorldView, vec4(a_normal, 0.0)).xyz);
    #ifdef TEX
        v_texcoord0 = a_texcoord0;
    #endif
    gl_Position = mul(matWorldViewProj, pos);
    #ifdef CLASSIC_LIGHT
        gl_Position.z = max(gl_Position.z, 0.00006103515625); // clamp lights to near clip plane to avoid them being partially clipped // 0.00006103515625 due to 16bit half float
    #endif
}
