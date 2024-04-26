$input a_position, a_normal, a_texcoord0
#ifdef STEREO
$output v_worldPos, v_tablePos, v_normal, v_texcoord0, v_eye
#else
$output v_worldPos, v_tablePos, v_normal, v_texcoord0
#endif

#include "common.sh"

uniform mat4 matWorld;
uniform mat4 matView;
uniform mat4 matWorldView;
uniform mat4 matWorldViewInverseTranspose;
#ifdef STEREO
	uniform mat4 matWorldViewProj[2];
#else
	uniform mat4 matWorldViewProj;
#endif


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
	#ifdef STEREO
		gl_Position = mul(matWorldViewProj[gl_InstanceID], pos);
		gl_Layer = gl_InstanceID;
		v_eye = gl_InstanceID;
	#else
		gl_Position = mul(matWorldViewProj, pos);
	#endif
    #ifdef CLASSIC_LIGHT
        gl_Position.z = max(gl_Position.z, 0.00006103515625); // clamp lights to near clip plane to avoid them being partially clipped // 0.00006103515625 due to 16bit half float
    #endif
}
