// license:GPLv3+

$input a_position, a_texcoord0
#ifdef STEREO
$output v_texcoord0, v_eye
uniform vec4 layer;
#else
$output v_texcoord0
#endif

#include "common.sh"

uniform vec4 w_h_height;

void main()
{
   gl_Position = vec4(a_position.x, a_position.y, 0.0, 1.0);
   #ifdef STEREO
      gl_Layer = gl_InstanceID;
      v_eye = layer.x + gl_InstanceID;
   #endif
   
   // Input texture coordinate is [0..1], we want it to be exact sample position ranging from [0.5/size .. (size - 0.5)/size]
   v_texcoord0.x = a_texcoord0.x * (1.0 - w_h_height.x) + 0.5 * w_h_height.x;
   #if TEX_V_IS_UP
      v_texcoord0.y = (1.0 - a_texcoord0.y) * (1.0 - w_h_height.y) + 0.5 * w_h_height.y;
   #else
      v_texcoord0.y = a_texcoord0.y * (1.0 - w_h_height.y) + 0.5 * w_h_height.y;
   #endif
}
