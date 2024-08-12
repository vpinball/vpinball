// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"

uniform vec4 w_h_height; // w_h_height.xy contains inverse size of source texture (1/w, 1/h)

SAMPLER2DSTEREO(tex_ao,              3); // Ambient Occlusion

void main()
{
   float result = FBGamma(saturate(texStereoNoLod(tex_ao, v_texcoord0 - 0.5*w_h_height.xy).x)); // shift half a texel to blurs over 2x2 window
   gl_FragColor = vec4(result, result, result, 1.0);
}
