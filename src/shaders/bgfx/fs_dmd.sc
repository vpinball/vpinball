// license:GPLv3+

$input v_texcoord0, v_texcoord1
#ifdef CLIP
	$input v_clipDistance
#endif

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
   vec3 color = vColor_Intensity.xyz; //!! create function that resembles LUT from VPM?
   if(rgba.a != 0.0)
      color *= rgba.rgb;
   else
      color *= rgba.r * (255.9 / 100.);

   return vec4(InvGamma(color), vRes_Alpha_time.z);
}
#endif

#if 0
float nrand(const vec2 uv)
{
   return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}
#endif

vec2 hash22(const vec2 uv)
{
   vec3 p3 = fract(uv.xyx * vec3(.1031, .1030, .0973));
   p3 += dot(p3, p3.yzx + 33.33);
   return fract((p3.xx + p3.yz)*p3.zy);
}

#if 0
float gold_noise(const vec2 xy, const float seed)
{
   return fract(tan(distance(xy * 1.61803398874989484820459, xy) * seed) * xy.x); // tan is usually slower than sin/cos
}
#endif

float triangularPDF(const float r) // from -1..1, c=0 (with random no r=0..1)
{
   float p = 2.*r;
   const bool b = (p > 1.);
   if (b)
      p = 2.-p;
   p = 1.-sqrt(p); //!! handle 0 explicitly due to compiler doing 1/inversesqrt(0)? but might be still 0 according to spec, as rsqrt(0) = inf and 1/inf = 0, but values close to 0 could be screwed up still
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

#if !defined(CLIP) && !defined(TEX)
EARLY_DEPTH_STENCIL
#endif
void main()
{
	#ifdef CLIP
	if (v_clipDistance < 0.0)
	   discard;
	#endif

	//!! this is incredibly heavy for a supposedly simple DMD output shader, but then again this is pretty robust for all kinds of scales and input resolutions now, plus also for 'distorted' output (via the flashers)!
	//!! gaussianPDF is even more heavy, introduces more noise and is only barely higher quality (=bit less moiree) 
	#ifdef DMD
	   const float blur = /*gaussian: 4.0; /*/ 1.5; // 1.0..2.0 looks best (between sharp and blurry), and 1.5 matches the intention of the triangle filter (see triangularPDF calls below)!
	   const vec2 ddxs = dFdx(v_texcoord0)*blur; // use ddx and ddy to help the oversampling below/make filtering radius dependent on projected 'dots'/texel
	   const vec2 ddys = dFdy(v_texcoord0)*blur;

	   const float dist_factor = clamp((1.-length(ddxs+ddys)*6.66) /* *sqrt(128./vRes_Alpha_time.x)*/, 0.4,1.0); // fades the smooth dots to unicolored rectangles for less aliasing

	   const vec2 offs = hash22(v_texcoord0 + vRes_Alpha_time.w); //vec2(nrand(tex0 + vRes_Alpha_time.w), nrand(tex0 + (2048.0 + vRes_Alpha_time.w))); // random offset for the oversampling

	   // brute force oversampling of DMD-texture and especially the dot-function (using 25 samples)
	   vec3 color2 = vec3_splat(0.);

	   const int samples = 13; //4,8,9,13,21,25,32 korobov,fibonacci
	   const float samples_float = float(samples);
	   UNROLL for (int i = 0; i < samples; ++i) // oversample the dots
	   {
		  const float i_float = float(i);
		  const vec2 xi = vec2(fract(i_float * (1.0 / samples_float) + offs.x), fract(i_float * (8.0 / samples_float) + offs.y)); //1,5,2,8,13,7,7 korobov,fibonacci
		  //const vec2 gxi = gaussianPDF(xi);
		  const vec2 uv = v_texcoord0 + /*gxi.x*ddxs + gxi.y*ddys; /*/ triangularPDF(xi.x)*ddxs + triangularPDF(xi.y)*ddys; //!! lots of ALU

		  const vec4 rgba = texNoLod(tex_dmd, uv); //!! lots of tex access by doing this all the time, but (tex) cache should be able to catch all of it

		  // simulate dot within the sampled texel
		  const vec2 dist = (fract(uv*vRes_Alpha_time.xy)*2.2 - 1.1) * dist_factor;
		  const float d = smoothstep(0., 1., 1.0 - sqr(dist.x*dist.x + dist.y*dist.y));

		  if (vColor_Intensity.w != 0.0)
			 color2 += rgba.rgb * d;
		  else
			 color2 += rgba.r * d;
	   }
	   color2 *= vColor_Intensity.xyz * ((1./samples_float) * sqr(dist_factor)); //!! create function that resembles LUT from VPM?

	   /*vec3 colorg = vec3(0,0,0);
	   UNROLL for(int j = -1; j <= 1; ++j)
	   UNROLL for(int i = -1; i <= 1; ++i)
	   {
		  //collect glow from neighbors
	   }*/

	   //if (rgba.r > 200.0)
	   //   gl_FragColor = vec4(InvGamma(min(color2,vec3(1.5,1.5,1.5))/*+colorg*/), 0.5);
	   //else
	   //gl_FragColor = vec4(InvGamma(color2/*+colorg*/), vRes_Alpha_time.z);

	   // Do not apply InvGamma anymore since it is already applied to DMD texture (BW) or during sampling (RGB), i.e. shading is now done in linear color space
	   gl_FragColor = vec4(color2/*+colorg*/, vRes_Alpha_time.z);

	#else // No DMD (sprite rendering)
		#ifdef TEX
			const vec4 l = texture2D(tex_sprite, v_texcoord0);
			if (l.a < alphaTestValue.x)
				discard; //stop the pixel shader if alpha test should reject pixel to avoid writing to the depth buffer
			gl_FragColor = vec4(/*InvGamma*/(l.rgb * vColor_Intensity.rgb * vColor_Intensity.a), l.a);
		#else
			gl_FragColor = vec4(InvGamma(vColor_Intensity.xyz * vColor_Intensity.w), 1.0);
		#endif

	#endif
}
