$input v_texcoord0

#include "bgfx_shader.sh"
#include "common.sh"

uniform vec4 vColor_Intensity;
uniform vec4 vRes_Alpha_time;
uniform vec4 alphaTestValue; // Actually float but extended to vec4 for BGFX (and we should use the builtin)


#ifdef DMD
SAMPLER2D(tex_dmd, 0); // DMD
#else
SAMPLER2D(tex_sprite, 0); // Sprite
#endif


//
// PS functions (DMD and "sprites")
//

#if 0 // raw pixelated output
vec4 ps_main_DMD_no(const in VS_OUTPUT IN) : COLOR
{
   const vec4 rgba = texNoLod(tex_dmd, v_texcoord0);
   vec3 color = vColor_Intensity.xyz * vColor_Intensity.w; //!! create function that resembles LUT from VPM?
   if(rgba.a != 0.0)
      color *= rgba.rgb;
   else
      color *= rgba.r * (255.9 / 100.);

   return vec4(InvToneMap(InvGamma(color)), vRes_Alpha_time.z); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
}
#endif

float nrand(const vec2 uv)
{
   return frac(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

#if 0
float gold_noise(const vec2 xy, const float seed)
{
   return frac(tan(distance(xy * 1.61803398874989484820459, xy) * seed) * xy.x); // tan is usually slower than sin/cos
}
#endif

float triangularPDF(const float r) // from -1..1, c=0 (with random no r=0..1)
{
   float p = 2.*r;
   const bool b = (p > 1.);
   if (b)
      p = 2.-p;
   p = 1.-sqrt(p); //!! handle 0 explicitly due to compiler doing 1/rsqrt(0)? but might be still 0 according to spec, as rsqrt(0) = inf and 1/inf = 0, but values close to 0 could be screwed up still
   return b ? p : -p;
}

#if 0
// approximation, mainly to get limited support (i.e. not infinite, like real gauss, which is nonsense for a small amount of samples)
vec2 gaussianPDF(const vec2 xi)
{
   vec2 u;
   sincos(6.283185307179586476925286766559 * xi.y, u.x, u.y);
   const float root4 = sqrt(sqrt(1.0 - xi.x));
   const float half_r = sqrt(0.25 - 0.25 * root4);
   return u * half_r;
}
#endif

//!! this is incredibly heavy for a supposedly simple DMD output shader, but then again this is pretty robust for all kinds of scales and input resolutions now, plus also for 'distorted' output (via the flashers)!
//!! gaussianPDF is even more heavy, introduces more noise and is only barely higher quality (=bit less moiree) 
void main()
{
#ifdef DMD
   const float blur = /*gaussian: 4.0; /*/ 1.5; // 1.0..2.0 looks best (between sharp and blurry), and 1.5 matches the intention of the triangle filter (see triangularPDF calls below)!
   const vec2 ddxs = dFdx(v_texcoord0)*blur; // use ddx and ddy to help the oversampling below/make filtering radius dependent on projected 'dots'/texel
   const vec2 ddys = dFdy(v_texcoord0)*blur;

   const vec2 offs = vec2(nrand(v_texcoord0 + vRes_Alpha_time.w), nrand(v_texcoord0 + (2048.0 + vRes_Alpha_time.w))); // random offset for the oversampling

   // brute force oversampling of DMD-texture and especially the dot-function (using 25 samples)
   vec3 color2 = vec3(0., 0., 0.);

   const int samples = 21; //4,8,9,13,21,25,32 korobov,fibonacci
   UNROLL for (int i = 0; i < samples; ++i) // oversample the dots
   {
      const vec2 xi = vec2(frac(i* (1.0 / samples) + offs.x), frac(i* (13.0 / samples) + offs.y)); //1,5,2,8,13,7,7 korobov,fibonacci
      //const vec2 gxi = gaussianPDF(xi);
      const vec2 uv = v_texcoord0 + /*gxi.x*ddxs + gxi.y*ddys; /*/ triangularPDF(xi.x)*ddxs + triangularPDF(xi.y)*ddys; //!! lots of ALU

      const vec4 rgba = texture2DLod(tex_dmd, uv, 0.0); //!! lots of tex access by doing this all the time, but (tex) cache should be able to catch all of it

      // simulate dot within the sampled texel
      const vec2 dist = frac(uv*vRes_Alpha_time.xy)*2.2 - 1.1;
      const float d = smoothstep(0., 1., 1.0 - sqr(dist.x*dist.x + dist.y*dist.y));

      if (rgba.a != 0.0)
         color2 += rgba.rgb * d;
      else
         color2 += rgba.r * (255.9 / 100.) * d;
   }
   color2 *= vColor_Intensity.xyz * (vColor_Intensity.w/samples); //!! create function that resembles LUT from VPM?

   /*vec3 colorg = vec3(0,0,0);
   UNROLL for(int j = -1; j <= 1; ++j)
   UNROLL for(int i = -1; i <= 1; ++i)
   {
      //collect glow from neighbors
   }*/

   //if (rgba.r > 200.0)
   //   gl_FragColor = vec4(InvToneMap(InvGamma(min(color2,vec3(1.5,1.5,1.5))/*+colorg*/)), 0.5);
   //else
   gl_FragColor = vec4(InvToneMap(InvGamma(color2/*+colorg*/)), vRes_Alpha_time.z); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
 
#else
   const vec4 l = texture2D(tex_sprite, v_texcoord0);
   if (l.a < alphaTestValue.x)
      discard; //stop the pixel shader if alpha test should reject pixel to avoid writing to the depth buffer
   gl_FragColor = vec4(InvToneMap(/*InvGamma*/(l.xyz * vColor_Intensity.xyz * vColor_Intensity.w)), l.w); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
#endif
}

/*
vec4 ps_main_noDMD_notex(const in VS_OUTPUT IN) : COLOR
{
   gl_FragColor = vec4(InvToneMap(InvGamma(vColor_Intensity.xyz * vColor_Intensity.w)), 1.0); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
}
*/
