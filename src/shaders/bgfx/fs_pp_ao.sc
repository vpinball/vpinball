// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"

uniform vec4 w_h_height; // w_h_height.xy contains inverse size of source texture (1/w, 1/h)

#ifdef STEREO
  SAMPLER2DSTEREO(tex_fb_unfiltered, 0); // Framebuffer
  #define tex_fb tex_fb_unfiltered
#else
  SAMPLER2DSTEREO(tex_fb_filtered,   0); // Framebuffer
  #define tex_fb tex_fb_filtered
#endif
SAMPLER2DSTEREO(tex_ao,              3); // Ambient Occlusion

void main()
{
   // v_texcoord0 is pixel perfect sampling which here means between the pixels resulting from supersampling (for 2x, it's in the middle of the 2 texels)
   gl_FragColor = vec4(texStereoNoLod(tex_fb, v_texcoord0                    ).rgb // moving AO before tonemap does not really change the look
                     * texStereoNoLod(tex_ao, v_texcoord0 - 0.5*w_h_height.xy).x   // shift half a texel to blurs over 2x2 window
                     , 1.0);
}
