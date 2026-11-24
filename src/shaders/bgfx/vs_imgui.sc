// license:GPLv3+

$input a_position, a_texcoord0, a_color0
$output v_color0, v_texcoord0, v_texcoord1

#include "common.sh"

#ifdef STEREO
uniform mat4 matWorldView[2];
uniform vec4 layer;
#else
uniform mat4 matWorldView;
#endif

void main()
{
    v_texcoord1 = a_position.xy;

    #ifdef STEREO
        // Target eye: the instance index for layered rendering, or the activated layer
        // index when the backend renders each layer through a dedicated pass
        float eye = layer.x + float(gl_InstanceID);
        gl_Layer = gl_InstanceID;
        #define mWorldView matWorldView[int(round(eye))]
    #else
        #define mWorldView matWorldView
    #endif
    // Full transform: an ortho mapping for display space UI, or a world space
    // mapping (perspective divide included) when drawn inside the VR projection
    vec4 pos = mul(mWorldView, vec4(a_position.xy, 0.0, 1.0));
    gl_Position = pos;

    v_texcoord0 = a_texcoord0;

    v_color0 = a_color0;
}
