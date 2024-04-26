$input a_position
$output v_tablePos

#include "common.sh"

#ifdef STEREO
	uniform mat4 matWorldViewProj[2];
#else
	uniform mat4 matWorldViewProj;
#endif

void main()
{
    v_tablePos = a_position.xyz;
	#ifdef STEREO
		gl_Position = mul(matWorldViewProj[gl_InstanceID], vec4(a_position, 1.0));
		gl_Layer = gl_InstanceID;
	#else
		gl_Position = mul(matWorldViewProj, vec4(a_position, 1.0));
	#endif
    //Disabled since this would move backdrop lights behind other backdrop parts (rendered at z = 0), this could be readded with a max at 0, but I don't get why we would need this type of hack
    //gl_Position.z = max(gl_Position.z, 0.00006103515625); // clamp lights to near clip plane to avoid them being partially clipped // 0.00006103515625 due to 16bit half float
}
