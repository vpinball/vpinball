// license:GPLv3+

#ifdef STEREO
$input v_texcoord0, v_eye
#else
$input v_texcoord0
#endif

#include "common.sh"

//
// Gaussian Blur Kernels
//
// Separable (approximate) Gaussian blur kernels, using GPU linear sampling to perform 2 samples in a single tex fetch (similar to https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/)
// Use e.g. https://observablehq.com/@s4l4x/efficient-gaussian-blur-with-linear-sampling to compute centered coefficients/offsets
// Use e.g. https://www.intel.com/content/www/us/en/developer/articles/technical/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms.html to compue not centered coefficients/offsets

SAMPLER2DSTEREO(tex_fb_filtered,  0);

// w_h_height.xy contains inverse size of source texture (1/w, 1/h), i.e. one texel shift to the upper (DX)/lower (OpenGL) left texel. Since OpenGL has upside down textures it leads to a different texel if not sampled on both sides
// . for bloom, w_h_height.z keeps strength
// . for mirror, w_h_height.z keeps inverse strength
// . for AO, w_h_height.zw keeps per-frame offset for temporal variation of the pattern
// . for AA techniques, w_h_height.z keeps source texture width, w_h_height.w is a boolean set to 1 when depth is available
// . for parallax stereo, w_h_height.z keeps source texture height, w_h_height.w keeps the 3D offset
uniform vec4 w_h_height; 

void main()
{
#if defined(BLUR_7)
   #define NO_CENTER
   #define ARRAY_SIZE 2
   const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.53473, 2.05896 ARRAY_END();
   const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.45134, 0.04866 ARRAY_END();

#elif defined(BLUR_9)
   #define ARRAY_SIZE 3
   const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.0, 1.3846153846153846, 3.2307692307692304 ARRAY_END();
   const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.22697126013264554, 0.31613854089904203, 0.070253009088676 ARRAY_END();

#elif defined(BLUR_11)
   #define NO_CENTER
   #define ARRAY_SIZE 3
   const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.62195, 2.27357, 4.14706 ARRAY_END();
   const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.32993, 0.15722, 0.01285 ARRAY_END();

#elif defined(BLUR_13)
   #define ARRAY_SIZE 4
   const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.0, 1.4117647058823528, 3.2941176470588234, 5.176470588235294 ARRAY_END();
   const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.1964795505549364, 0.2969024319496817, 0.09446895562035326, 0.010381203914324535 ARRAY_END();

#elif defined(BLUR_15)
   #define NO_CENTER
   #define ARRAY_SIZE 4
   const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.64417, 2.37795, 4.28970, 6.21493 ARRAY_END();
   const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.25044, 0.19233, 0.05095, 0.00628 ARRAY_END();

#elif defined(BLUR_19)
   #define NO_CENTER
   #define ARRAY_SIZE 5
   ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.65323, 2.42572, 4.36847, 6.31470, 8.26547 ARRAY_END();
   ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.19923, 0.18937, 0.08396, 0.02337, 0.00408 ARRAY_END();

#elif defined(BLUR_19H)
   #define NO_CENTER
   #define ARRAY_SIZE 5
   const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.63918, 2.35282, 4.25124, 6.17117, 8.11278 ARRAY_END();
   const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.27233, 0.18690, 0.03767, 0.00301, 0.00009 ARRAY_END();

#elif defined(BLUR_23)
   #define NO_CENTER
   #define ARRAY_SIZE 6
   const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.64851, 2.40054, 4.32612, 6.25954, 8.20247, 10.15531 ARRAY_END();
   const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.22791, 0.19358, 0.06543, 0.01187, 0.00115, 0.00006 ARRAY_END();

#elif defined(BLUR_27)
   #define NO_CENTER
   #define ARRAY_SIZE 7
   // 'h'-versions:  (smoother fadeout on edges)
   const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.20227,0.19003,0.08192,0.02181,0.00358,0.00036,0.00002 ARRAY_END();
   const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.65275,2.42313,4.36403,6.30877,8.25848,10.21386,12.17512 ARRAY_END();
   // org-version
   //const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.66025, 2.46412, 4.43566, 6.40762, 8.38017, 10.35347, 12.32765 ARRAY_END();
   //const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.14096, 0.15932, 0.10714, 0.05743, 0.02454, 0.00835, 0.00227 ARRAY_END();

#elif defined(BLUR_39)
   #define NO_CENTER
   #define ARRAY_SIZE 10
   // smoother fadeout on edges
   const ARRAY_BEGIN(float, offset, ARRAY_SIZE) 0.66063, 2.46625, 4.43946, 6.41301, 8.38706, 10.36173, 12.33715, 14.31341, 16.29062, 18.26884 ARRAY_END();
   const ARRAY_BEGIN(float, weight, ARRAY_SIZE) 0.13669, 0.15600, 0.10738, 0.05971, 0.02682, 0.00973, 0.00285, 0.00067, 0.00013, 0.00002 ARRAY_END();

#endif

#ifdef HORIZONTAL
   #define OFS vec2(w_h_height.x*offset[i], 0.0)
#else
   #define OFS vec2(0.0, w_h_height.y*offset[i])
#endif

#ifdef NO_CENTER
   vec3 result = vec3(0.0, 0.0, 0.0);
   UNROLL for(int i = 0; i < ARRAY_SIZE; ++i)
      result += (texStereoNoLod(tex_fb_filtered, v_texcoord0.xy + OFS).rgb
                +texStereoNoLod(tex_fb_filtered, v_texcoord0.xy - OFS).rgb) * weight[i];
#else
   vec3 result = texStereoNoLod(tex_fb_filtered, v_texcoord0.xy      ).rgb  * weight[0];
   UNROLL for(int i = 1; i < ARRAY_SIZE; ++i)
      result += (texStereoNoLod(tex_fb_filtered, v_texcoord0.xy + OFS).rgb
                +texStereoNoLod(tex_fb_filtered, v_texcoord0.xy - OFS).rgb) * weight[i];
#endif

   gl_FragColor = vec4(result, 1.0);
}
