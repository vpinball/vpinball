$input v_texcoord0

#include "bgfx_shader.sh"
#include "common.sh"


// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height;

uniform vec2 AO_scale_timeblur;

uniform vec4 color_grade; // converted to vec4 for BGFX
uniform vec4 do_dither; // converted to vec4 for BGFX
uniform vec4 do_bloom; // converted to vec4 for BGFX

#ifdef GRAY
#define rtype float
#define swizzle r
#elif defined(BW)
#define rtype vec2
#define swizzle rg
#else
#define rtype vec3
#define swizzle rgb
#endif

#ifdef NOFILTER
#define tex_fb tex_fb_unfiltered
SAMPLER2D(tex_fb_unfiltered,  0); // Framebuffer
#else
#define tex_fb tex_fb_filtered
SAMPLER2D(tex_fb_filtered,  0); // Framebuffer
#endif

SAMPLER2D(tex_bloom,  1); // Bloom
SAMPLER2D(tex_color_lut,  2); // Color grade
SAMPLER2D(tex_ao,  3); // Ambient Occlusion
SAMPLER2D(tex_depth,  4); // Depth Buffer
SAMPLER2D(tex_ao_dither,  5); // Ambient Occlusion Dither



vec3 blue_gauss_noise(const vec2 c1, const vec3 rgb)
{
   // https://www.shadertoy.com/view/XljyRR
   //vec2 c0 = vec2(c1.x - 1., c1.y);
   //vec2 c2 = vec2(c1.x + 1., c1.y);
   const vec3 cx = vec3(c1.x - 1., c1.x, c1.x + 1.);
   const vec4 f0 = fract(vec4(cx * 9.1031, c1.y * 8.1030));
   const vec4 f1 = fract(vec4(cx * 7.0973, c1.y * 6.0970));
   const vec4 t0 = vec4(f0.xw,f1.xw); //fract(c0.xyxy * vec4(.1031,.1030,.0973,.0970));
   const vec4 t1 = vec4(f0.yw,f1.yw); //fract(c1.xyxy * vec4(.1031,.1030,.0973,.0970));
   const vec4 t2 = vec4(f0.zw,f1.zw); //fract(c2.xyxy * vec4(.1031,.1030,.0973,.0970));
   const vec4 p0 = t0 + dot(t0, t0.wzxy + 19.19);
   const vec4 p1 = t1 + dot(t1, t1.wzxy + 19.19);
   const vec4 p2 = t2 + dot(t2, t2.wzxy + 19.19);
   const vec4 n0 = fract(p0.zywx * (p0.xxyz + p0.yzzw));
   const vec4 n1 = fract(p1.zywx * (p1.xxyz + p1.yzzw));
   const vec4 n2 = fract(p2.zywx * (p2.xxyz + p2.yzzw));
   const vec4 r4 = 0.5*n1 - 0.125*(n0 + n2);
   const float  r  = r4.x+r4.y+r4.z+r4.w - 0.5;

   const float quantSteps = 256.0; //!! how to choose/select this for 5/6/5bit??
   return rgb + vec3(r,-r,r) * (1.0/(quantSteps - 1.0));
}

vec3 FBDither(const vec3 color, /*const int2 pos*/const vec2 tex0)
{
   BRANCH if (!(do_dither.x > 0.))
       return color;

   //return color + bayer_dither_pattern[pos.x%8][pos.y%8];

#ifndef BLUE_NOISE_DITHER
   //return DitherVlachos(floor(tex0*(1.0/w_h_height.xy)+0.5) + w_h_height.z/*w*/, color);       // note that w_h_height.w is the same nowadays
   return blue_gauss_noise(floor(tex0*(1.0/w_h_height.xy)+0.5) + 0.07*w_h_height.z/*w*/, color); // dto.
#else // needs tex_ao_dither
   const float quantSteps = 256.0; //!! how to choose/select this for 5/6/5bit??

   // TPDF:
   const vec3 dither = /*vec3(GradientNoise(tex0 / w_h_height.xy + w_h_height.zw*3.141),
      GradientNoise(tex0.yx / w_h_height.yx + w_h_height.zw*1.618),
      GradientNoise(tex0 / w_h_height.xy + w_h_height.wz*2.718281828459));*/
   // Dither64(tex0 / w_h_height.xy+w_h_height.zw, 0.);
       texture2DLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.zw*3.141, 0.0).xyz;
   return color + triangularPDF(dither) * (1.0/quantSteps); // quantSteps-1. ?

   /*const vec3 dither = texture2DLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.zw*2.718281828459, 0.0).xyz;
   const vec3 dither2 = texture2DLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.wz*3.14159265358979, 0.0).xyz;
   return color + (dither - dither2) / quantSteps;*/

   //const vec3 dither = texture2DLod(tex_ao_dither, tex0 / (64.0*w_h_height.xy) + w_h_height.wz*3.141, 0.0).xyz*2.0 - 1.0;

   // Lottes (1st one not working, 2nd 'quality', 3rd 'tradeoff'), IMHO too much magic:
   /*const float blackLimit = 0.5 * InvGamma(1.0/(quantSteps - 1.0));
   const float amount = 0.75 * (InvGamma(1.0/(quantSteps - 1.0)) - 1.0);
   return color + dither*min(color + blackLimit, amount);*/

   //const vec3 amount = InvGamma(FBGamma(color) + (4. / quantSteps)) - color;
   
   //const float luma = saturate(dot(color,vec3(0.212655,0.715158,0.072187)));
   //const vec3 amount = lerp(
   //  InvGamma(4. / quantSteps), //!! precalc? would also solve 5/6/5bit issue!
   //  InvGamma((4. / quantSteps)+1.)-1.,
   //  luma);

   //return color + dither*amount;

   // RPDF:
   //return color + dither/quantSteps; // use dither texture instead nowadays // 64 is the hardcoded dither texture size for AOdither.bmp
#endif
}

vec2 FBDither(const vec2 color, /*const int2 pos*/const vec2 tex0)
{
   return color; // on RG-only rendering do not dither anything for performance
}

float FBDither(const float color, /*const int2 pos*/const vec2 tex0)
{
   return color; // on R-only rendering do not dither anything for performance
}

//

vec3 FBColorGrade(vec3 color)
{
   BRANCH if (!(color_grade.x > 0.))
       return color;

   color.xy = color.xy*(15.0/16.0) + 1.0/32.0; // assumes 16x16x16 resolution flattened to 256x16 texture
   color.z *= 15.0;

   const float x = (color.x + floor(color.z))/16.0;
   const vec3 lut1 = texture2DLod(tex_color_lut, vec2(x,          color.y), 0.0).xyz; // two lookups to blend/lerp between blue 2D regions
   const vec3 lut2 = texture2DLod(tex_color_lut, vec2(x+1.0/16.0, color.y), 0.0).xyz;
   return mix(lut1,lut2, fract(color.z));
}



void main()
{
   // v_texcoord0 is pixel perfect sampling which here means between the pixels resulting from supersampling (for 2x, it's in the middle of the 2 texels)
   
   rtype result = texture2DLod(tex_fb, v_texcoord0, 0.0).swizzle;
   
   // moving AO before tonemap does not really change the look
   #ifdef AO
   result *= texture2DLod(tex_ao, v_texcoord0 - 0.5*w_h_height.xy, 0.0).x; // shift half a texel to blurs over 2x2 window
   #endif

   BRANCH if (do_bloom.x > 0.)
      result += texture2DLod(tex_bloom, v_texcoord0, 0.0).swizzle; //!! offset?

 
   #ifdef GRAY
   
   #ifdef TONEMAP
   result = FBGamma(saturate(FBDither(result, v_texcoord0)));
   #endif
   gl_FragColor = vec4(result, result, result, 1.0);
   
   #elif defined(BW)
   
   #ifdef TONEMAP
   float grey = FBGamma(saturate(dot(FBDither(result, v_texcoord0), vec2(0.176204+0.0108109*0.5,0.812985+0.0108109*0.5))));
   #else
   float grey = dot(result, vec2(0.176204+0.0108109*0.5,0.812985+0.0108109*0.5));
   #endif
   gl_FragColor = vec4(grey, grey, grey, 1.0);
   
   #else
   
   #ifdef TONEMAP
   result = FBColorGrade(FBGamma(saturate(FBDither(result, v_texcoord0))));
   #endif
   gl_FragColor = vec4(result, 1.0);
   
   #endif
}
