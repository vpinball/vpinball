//!! split into pure sprite & pure DMD?

#include "Helpers.fxh"

float4 vColor_Intensity;
float4 vRes_Alpha_time;

texture Texture0;

sampler2D texSampler0 : TEXUNIT0 = sampler_state // DMD
{
	Texture	  = (Texture0);
    MIPFILTER = NONE;
    MAGFILTER = POINT;
    MINFILTER = POINT;
    // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
	ADDRESSU  = MIRROR;
	ADDRESSV  = MIRROR;
};

sampler2D texSampler1 : TEXUNIT0 = sampler_state // Sprite
{
	Texture	  = (Texture0);
    MIPFILTER = LINEAR;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
    // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
	ADDRESSU  = MIRROR;
	ADDRESSV  = MIRROR;
	SRGBTexture = true;
};

//
// VS function output structures 
//

struct VS_OUTPUT 
{ 
   float4 pos  : POSITION;
   float2 tex0 : TEXCOORD0;
}; 

VS_OUTPUT vs_main (in float4 vPosition : POSITION0,
                   in float2 tc        : TEXCOORD0)
{ 
   VS_OUTPUT Out;

   Out.pos = float4(vPosition.xy, 0.0,1.0);
   Out.tex0 = tc;
   
   return Out;
}

// transformation matrices (only used for flashers and backbox so far)
float4x4 matWorldViewProj : WORLDVIEWPROJ;

VS_OUTPUT vs_simple_world(in float4 vPosition : POSITION0,
                          in float2 tc : TEXCOORD0)
{
    VS_OUTPUT Out;

    Out.pos = mul(vPosition, matWorldViewProj);
    Out.tex0 = tc;

    return Out;
}

//
// PS functions (DMD and "sprites")
//

#if 0 // raw pixelated output
float4 ps_main_DMD_no(in VS_OUTPUT IN) : COLOR
{
   const float4 rgba = tex2Dlod(texSampler0, float4(IN.tex0, 0.,0.));
   float3 color = vColor_Intensity.xyz * vColor_Intensity.w; //!! create function that resembles LUT from VPM?
   if(rgba.a != 0.0)
      color *= rgba.bgr;
   else
      color *= rgba.b * (255.9 / 100.);

   return float4(InvToneMap(InvGamma(color)), vRes_Alpha_time.z); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
}
#endif

float nrand(const float2 uv)
{
   return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

#if 0
float gold_noise(const float2 xy, const float seed)
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
float2 gaussianPDF(const float2 xi)
{
   float2 u;
   sincos(6.283185307179586476925286766559 * xi.y, u.x, u.y);
   const float root4 = sqrt(sqrt(1.0 - xi.x));
   const float half_r = sqrt(0.25 - 0.25 * root4);
   return u * half_r;
}
#endif

//!! this is incredibly heavy for a supposedly simple DMD output shader, but then again this is pretty robust for all kinds of scales and input resolutions now, plus also for 'distorted' output (via the flashers)!
//!! gaussianPDF is even more heavy, introduces more noise and is only barely higher quality (=bit less moiree) 
float4 ps_main_DMD(in VS_OUTPUT IN) : COLOR
{
   const float blur = /*gaussian: 4.0; /*/ 1.5; // 1.0..2.0 looks best (between sharp and blurry), and 1.5 matches the intention of the triangle filter (see triangularPDF calls below)!
   const float2 ddxs = ddx(IN.tex0)*blur; // use ddx and ddy to help the oversampling below/make filtering radius dependent on projected 'dots'/texel
   const float2 ddys = ddy(IN.tex0)*blur;

   const float2 offs = float2(nrand(IN.tex0 + vRes_Alpha_time.w), nrand(IN.tex0 + (2048.0 + vRes_Alpha_time.w))); // random offset for the oversampling

   // brute force oversampling of DMD-texture and especially the dot-function (using 25 samples)
   float3 color2 = float3(0., 0., 0.);

   const int samples = 21; //4,8,9,13,21,25,32 korobov,fibonacci
   [unroll] for (int i = 0; i < samples; ++i) // oversample the dots
   {
      const float2 xi = float2(frac(i* (1.0 / samples) + offs.x), frac(i* (13.0 / samples) + offs.y)); //1,5,2,8,13,7,7 korobov,fibonacci
      //const float2 gxi = gaussianPDF(xi);
      const float2 uv = IN.tex0 + /*gxi.x*ddxs + gxi.y*ddys; /*/ triangularPDF(xi.x)*ddxs + triangularPDF(xi.y)*ddys; //!! lots of ALU

      const float4 rgba = tex2Dlod(texSampler0, float4(uv, 0., 0.)); //!! lots of tex access by doing this all the time, but (tex) cache should be able to catch all of it

      // simulate dot within the sampled texel
      const float2 dist = frac(uv*vRes_Alpha_time.xy)*2.2 - 1.1;
      const float d = smoothstep(0., 1., 1.0 - sqr(dist.x*dist.x + dist.y*dist.y));

      if (rgba.a != 0.0)
         color2 += rgba.bgr * d;
      else
         color2 += rgba.b * (255.9 / 100.) * d;
   }
   color2 *= vColor_Intensity.xyz * (vColor_Intensity.w/samples); //!! create function that resembles LUT from VPM?

   /*float3 colorg = float3(0,0,0);
   [unroll] for(int j = -1; j <= 1; ++j)
   [unroll] for(int i = -1; i <= 1; ++i)
   {
      //collect glow from neighbors
   }*/

   //if (rgba.b > 200.0)
   //   return float4(InvToneMap(InvGamma(min(color2,float3(1.5,1.5,1.5))/*+colorg*/)), 0.5);
   //else
   return float4(InvToneMap(InvGamma(color2/*+colorg*/)), vRes_Alpha_time.z); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
}

float4 ps_main_noDMD(in VS_OUTPUT IN) : COLOR
{
   const float4 l = tex2D(texSampler1, IN.tex0);

   return float4(InvToneMap(/*InvGamma*/(l.xyz * vColor_Intensity.xyz * vColor_Intensity.w)), l.w); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
}

float4 ps_main_noDMD_notex(in VS_OUTPUT IN) : COLOR
{
   return float4(InvToneMap(InvGamma(vColor_Intensity.xyz * vColor_Intensity.w)), 1.0); //!! meh, this sucks a bit performance-wise, but how to avoid this when doing fullscreen-tonemap/gamma without stencil and depth read?
}


technique basic_DMD
{ 
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader  = compile ps_3_0 ps_main_DMD();
   }
}

technique basic_DMD_world
{
    pass P0
    {
        VertexShader = compile vs_3_0 vs_simple_world();
        PixelShader  = compile ps_3_0 ps_main_DMD();
    }
}


technique basic_noDMD
{ 
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader  = compile ps_3_0 ps_main_noDMD();
   }
}

technique basic_noDMD_world
{ 
   pass P0
   {
      VertexShader = compile vs_3_0 vs_simple_world();
      PixelShader  = compile ps_3_0 ps_main_noDMD();
   }
}

technique basic_noDMD_notex
{ 
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main();
      PixelShader  = compile ps_3_0 ps_main_noDMD_notex();
   }
}
