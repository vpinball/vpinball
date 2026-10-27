// license:GPLv3+

$input a_position, a_normal, a_texcoord0
$output v_color0, v_texcoord0, v_texcoord1

#include "common.sh"

#ifdef STEREO
uniform mat4 matWorldView[2];
#else
uniform mat4 matWorldView;
#endif

uniform vec4 staticColor_Alpha;
#define stereoOfs staticColor_Alpha.x

void main()
{
    v_texcoord1 = a_position.xy;

    vec4 ofsPos = vec4(a_position.xy, 0.0, 1.0);
    #ifdef STEREO
        if (gl_InstanceID == 0)
            ofsPos.x += stereoOfs;
        else
            ofsPos.x -= stereoOfs;
        gl_Layer = gl_InstanceID;
    #endif
    #ifdef STEREO
        #define mWorldView matWorldView[gl_InstanceID]
    #else
        #define mWorldView matWorldView
    #endif
    vec4 pos = mul(mWorldView, ofsPos);
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);

    v_texcoord0 = a_texcoord0;

    v_color0 = vec4(a_normal.rgb, a_position.z);
}
