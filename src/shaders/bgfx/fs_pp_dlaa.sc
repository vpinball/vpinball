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

   const vec4 sampleCenter = sampleOffset(u, vec2(0.0, 0.0) );
   BRANCH if(w_h_height.w == 1.0 /*&& sampleCenter.a == 0.0*/) // depth buffer available? /*AND no edge here? -> ignored because of performance*/
   {
      const float depth0 = texStereoNoLod(tex_depth, u).x;
      BRANCH if((depth0 == 1.0) || (depth0 == 0.0)) // early out if depth too large (=BG) or too small (=DMD,etc)
      {
         gl_FragColor = vec4(sampleCenter.xyz, 1.0);
         return;
      }
   }

   // short edges
   const vec4 sampleHorizNeg0  = sampleOffset(u, vec2(-1.5,  0.0) );
   const vec4 sampleHorizPos0  = sampleOffset(u, vec2( 1.5,  0.0) );
   const vec4 sampleVertNeg0   = sampleOffset(u, vec2( 0.0, -1.5) );
   const vec4 sampleVertPos0   = sampleOffset(u, vec2( 0.0,  1.5) );

   const vec3 sumHoriz         = sampleHorizNeg0.xyz + sampleHorizPos0.xyz;
   const vec3 sumVert          = sampleVertNeg0.xyz  + sampleVertPos0.xyz;

   const vec3 sampleLeft       = sampleOffset(u, vec2(-1.0,  0.0) ).rgb;
   const vec3 sampleRight      = sampleOffset(u, vec2( 1.0,  0.0) ).rgb;
   const vec3 sampleTop        = sampleOffset(u, vec2( 0.0, -1.0) ).rgb;
   const vec3 sampleDown       = sampleOffset(u, vec2( 0.0,  1.0) ).rgb;

   const vec3 diffToCenterHoriz= abs((sampleLeft+sampleRight) * 0.5 - sampleCenter.xyz); //!! was sumHoriz instead of l&r
   const vec3 diffToCenterVert = abs((sampleTop+sampleDown) * 0.5 - sampleCenter.xyz); //!! was sumVert instead of t&d

   const float valueEdgeHoriz    = avg(diffToCenterHoriz);
   const float valueEdgeVert     = avg(diffToCenterVert);

   const float edgeDetectHoriz   = 3.0 * valueEdgeHoriz - 0.1; //!! magic params
   const float edgeDetectVert    = 3.0 * valueEdgeVert  - 0.1;

   const vec3 avgHoriz           = (sumHoriz + sampleCenter.xyz) * (1.0/3.0);
   const vec3 avgVert            = (sumVert  + sampleCenter.xyz) * (1.0/3.0);

   const float valueHoriz        = avg(avgHoriz);
   const float valueVert         = avg(avgVert);

   const float blurAmountHoriz   = saturate(edgeDetectHoriz / valueHoriz);
   const float blurAmountVert    = saturate(edgeDetectVert  / valueVert);

   vec3 aaResult                 = mix( sampleCenter.xyz, avgHoriz, blurAmountVert * 0.5); //!! magic sharpen
   aaResult                      = mix( aaResult,         avgVert,  blurAmountHoriz * 0.5); //!! magic sharpen

   // long edges
   const vec4 sampleVertNeg1   = sampleOffset(u, vec2(0.0, -3.5) );
   const vec4 sampleVertNeg15  = sampleOffset(u, vec2(0.0, -5.5) );
   const vec4 sampleVertNeg2   = sampleOffset(u, vec2(0.0, -7.5) );
   const vec4 sampleVertPos1   = sampleOffset(u, vec2(0.0,  3.5) );
   const vec4 sampleVertPos15  = sampleOffset(u, vec2(0.0,  5.5) );
   const vec4 sampleVertPos2   = sampleOffset(u, vec2(0.0,  7.5) );

   const vec4 sampleHorizNeg1  = sampleOffset(u, vec2(-3.5, 0.0) );
   const vec4 sampleHorizNeg15 = sampleOffset(u, vec2(-5.5, 0.0) );
   const vec4 sampleHorizNeg2  = sampleOffset(u, vec2(-7.5, 0.0) );
   const vec4 sampleHorizPos1  = sampleOffset(u, vec2( 3.5, 0.0) );
   const vec4 sampleHorizPos15 = sampleOffset(u, vec2( 5.5, 0.0) );
   const vec4 sampleHorizPos2  = sampleOffset(u, vec2( 7.5, 0.0) );

   const float pass1EdgeAvgHoriz = saturate(( sampleHorizNeg2.a + sampleHorizNeg1.a + sampleHorizNeg15.a + sampleHorizNeg0.a + sampleHorizPos0.a + sampleHorizPos1.a + sampleHorizPos15.a + sampleHorizPos2.a ) * (2.0 / 8.0) - 1.0);
   const float pass1EdgeAvgVert  = saturate(( sampleVertNeg2.a  + sampleVertNeg1.a + sampleVertNeg15.a  + sampleVertNeg0.a  + sampleVertPos0.a + sampleVertPos1.a + sampleVertPos15.a  + sampleVertPos2.a  ) * (2.0 / 8.0) - 1.0);

   BRANCH if(abs(pass1EdgeAvgHoriz - pass1EdgeAvgVert) > 0.2) //!! magic
   {
        const float valueHorizLong = avg(sampleHorizNeg2.xyz + sampleHorizNeg1.xyz + sampleHorizNeg15.xyz + sampleHorizNeg0.xyz + sampleHorizPos0.xyz + sampleHorizPos1.xyz + sampleHorizPos15.xyz + sampleHorizPos2.xyz) * (1.0/8.0);
        const float valueVertLong  = avg(sampleVertNeg2.xyz  + sampleVertNeg1.xyz + sampleVertNeg15.xyz + sampleVertNeg0.xyz  + sampleVertPos0.xyz + sampleVertPos1.xyz + sampleVertPos15.xyz + sampleVertPos2.xyz) * (1.0/8.0);

        const float valueCenter    = avg(sampleCenter.xyz);
        const float valueLeft      = avg(sampleLeft);
        const float valueRight     = avg(sampleRight);
        const float valueTop       = avg(sampleTop);
        const float valueBottom    = avg(sampleDown);

        const float vx = (valueCenter == valueLeft)   ? 0. : saturate(      ( valueVertLong  - valueLeft   ) / (valueCenter - valueLeft));
        const float hx = (valueCenter == valueTop)    ? 0. : saturate(      ( valueHorizLong - valueTop    ) / (valueCenter - valueTop));
        const float vy = (valueCenter == valueRight)  ? 0. : saturate(1.0 + ( valueVertLong  - valueCenter ) / (valueCenter - valueRight));
        const float hy = (valueCenter == valueBottom) ? 0. : saturate(1.0 + ( valueHorizLong - valueCenter ) / (valueCenter - valueBottom));

        const vec3 longBlurVert  = mix( sampleRight,
                                           mix( sampleLeft,  sampleCenter.xyz, vx ),
                                           vy );
        const vec3 longBlurHoriz = mix( sampleDown,
                                           mix( sampleTop,   sampleCenter.xyz, hx ),
                                           hy );

        aaResult                   = mix( aaResult, longBlurVert, pass1EdgeAvgVert * 0.5); //!! magic
        aaResult                   = mix( aaResult, longBlurHoriz, pass1EdgeAvgHoriz * 0.5); //!! magic

        //test: return vec4(aaResult,1.);
   }
   //test: return vec4(0,0,0,1);

   gl_FragColor = /*test: vec4(sampleCenter.a,sampleCenter.a,sampleCenter.a,1.0);*/ vec4(aaResult, 1.0);
}
