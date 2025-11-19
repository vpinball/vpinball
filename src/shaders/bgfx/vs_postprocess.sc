// license:GPLv3+

$input a_position, a_texcoord0
#ifdef STEREO
$output v_texcoord0, v_eye
uniform vec4 layer;
#else
$output v_texcoord0
#endif

#include "common.sh"

void main()
{
   gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
   #ifdef STEREO
      gl_Layer = gl_InstanceID;
      v_eye = layer.x + gl_InstanceID;
   #endif
   
   #if TEX_V_IS_UP
      v_texcoord0.x = a_texcoord0.x;
      v_texcoord0.y = 1.0 - a_texcoord0.y;
   #else
      v_texcoord0.xy = a_texcoord0.xy;
   #endif
}
