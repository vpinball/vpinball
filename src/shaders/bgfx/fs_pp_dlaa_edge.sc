// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"


// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

SAMPLER2DSTEREO(tex_fb_filtered,  0); // Framebuffer (filtered)
SAMPLER2DSTEREO(tex_depth,        4); // DepthBuffer


#define sampleOffset(u, pixelOffset) texStereoNoLod(tex_fb_filtered, u + pixelOffset * w_h_height.xy)

float avg(const vec3 l)
{
   //return dot(l, vec3(0.25,0.5,0.25)); // experimental, red and blue should not suffer too much
   return (l.x+l.y+l.z) * (1.0 / 3.0);
}

void main()
{
   const vec2 u = v_texcoord0;

   const vec3 sCenter    = sampleOffset(u, vec2( 0.0,  0.0) ).rgb;
   const vec3 sUpLeft    = sampleOffset(u, vec2(-0.5, -0.5) ).rgb;
   const vec3 sUpRight   = sampleOffset(u, vec2( 0.5, -0.5) ).rgb;
   const vec3 sDownLeft  = sampleOffset(u, vec2(-0.5,  0.5) ).rgb;
   const vec3 sDownRight = sampleOffset(u, vec2( 0.5,  0.5) ).rgb;

   const vec3 diff       = abs( (sUpLeft + sUpRight) + (sDownLeft + sDownRight) - sCenter * 4.0 );
   const float  edgeMask   = avg(diff) * 4.0; //!! magic

   gl_FragColor = /*test: vec4(edgeMask,edgeMask,edgeMask,1.0);*/ vec4(sCenter, edgeMask);
}
