#include "Helpers.fxh"

float4 ms_zpd_ya_td;
float4 w_h_height; // in bloom w_h_height.z keeps strength

float2 AO_scale_timeblur;
float mirrorFactor;

/*static*/ bool color_grade;

texture Texture0; // FB
texture Texture1; // Bloom
texture Texture3; // AO Result & DepthBuffer
texture Texture4; // AO Dither & Color grade

sampler2D texSampler3 : TEXUNIT2 = sampler_state // AO Result
{
	Texture	  = (Texture3);
    MIPFILTER = NONE; //!! ??
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

sampler2D texSamplerDepth : TEXUNIT2 = sampler_state // Depth
{
	Texture	  = (Texture3);
	MIPFILTER = NONE; //!! ??
	MAGFILTER = POINT;
	MINFILTER = POINT;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

sampler2D texSampler4 : TEXUNIT0 = sampler_state // Framebuffer (unfiltered)
{
	Texture	  = (Texture0);
    MIPFILTER = NONE; //!! ??
    MAGFILTER = POINT;
    MINFILTER = POINT;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

sampler2D texSampler5 : TEXUNIT0 = sampler_state // Framebuffer (filtered)
{
	Texture	  = (Texture0);
    MIPFILTER = NONE; //!! ??
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

sampler2D texSampler6 : TEXUNIT2 = sampler_state // Color grade LUT
{
	Texture	  = (Texture4);
    MIPFILTER = NONE;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

sampler2D texSamplerAOdither : TEXUNIT3 = sampler_state // AO dither
{
	Texture = (Texture4);
	MIPFILTER = NONE;
	MAGFILTER = POINT;
	MINFILTER = POINT;
	ADDRESSU = Wrap;
	ADDRESSV = Wrap;
};

sampler2D texSamplerBloom : TEXUNIT1 = sampler_state // Bloom
{
	Texture	  = (Texture1);
    MIPFILTER = NONE; //!! ??
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};

/*sampler2D texSamplerNormals : TEXUNIT1 = sampler_state // Normals (unfiltered)
{
	Texture	  = (Texture1);
    MIPFILTER = NONE; //!! ??
    MAGFILTER = POINT;
    MINFILTER = POINT;
	ADDRESSU  = Clamp;
	ADDRESSV  = Clamp;
};*/

sampler2D texSamplerMirror : TEXUNIT0 = sampler_state // base texture
{
   Texture = (Texture0);
   MIPFILTER = NONE; //!! ??
   MAGFILTER = LINEAR;
   MINFILTER = LINEAR;
   ADDRESSU  = Clamp;
   ADDRESSV  = Clamp;
};

struct VS_OUTPUT_2D
{ 
   float4 pos  : POSITION; 
   float2 tex0 : TEXCOORD0;
};

//
//
//

/*static const float bayer_dither_pattern[8][8] = {
    {( 0/64.0-0.5)/255.0, (32/64.0-0.5)/255.0, ( 8/64.0-0.5)/255.0, (40/64.0-0.5)/255.0, ( 2/64.0-0.5)/255.0, (34/64.0-0.5)/255.0, (10/64.0-0.5)/255.0, (42/64.0-0.5)/255.0},   
    {(48/64.0-0.5)/255.0, (16/64.0-0.5)/255.0, (56/64.0-0.5)/255.0, (24/64.0-0.5)/255.0, (50/64.0-0.5)/255.0, (18/64.0-0.5)/255.0, (58/64.0-0.5)/255.0, (26/64.0-0.5)/255.0},   
    {(12/64.0-0.5)/255.0, (44/64.0-0.5)/255.0, ( 4/64.0-0.5)/255.0, (36/64.0-0.5)/255.0, (14/64.0-0.5)/255.0, (46/64.0-0.5)/255.0, ( 6/64.0-0.5)/255.0, (38/64.0-0.5)/255.0},   
    {(60/64.0-0.5)/255.0, (28/64.0-0.5)/255.0, (52/64.0-0.5)/255.0, (20/64.0-0.5)/255.0, (62/64.0-0.5)/255.0, (30/64.0-0.5)/255.0, (54/64.0-0.5)/255.0, (22/64.0-0.5)/255.0},   
    {( 3/64.0-0.5)/255.0, (35/64.0-0.5)/255.0, (11/64.0-0.5)/255.0, (43/64.0-0.5)/255.0, ( 1/64.0-0.5)/255.0, (33/64.0-0.5)/255.0, ( 9/64.0-0.5)/255.0, (41/64.0-0.5)/255.0},   
    {(51/64.0-0.5)/255.0, (19/64.0-0.5)/255.0, (59/64.0-0.5)/255.0, (27/64.0-0.5)/255.0, (49/64.0-0.5)/255.0, (17/64.0-0.5)/255.0, (57/64.0-0.5)/255.0, (25/64.0-0.5)/255.0},
    {(15/64.0-0.5)/255.0, (47/64.0-0.5)/255.0, ( 7/64.0-0.5)/255.0, (39/64.0-0.5)/255.0, (13/64.0-0.5)/255.0, (45/64.0-0.5)/255.0, ( 5/64.0-0.5)/255.0, (37/64.0-0.5)/255.0},
    {(63/64.0-0.5)/255.0, (31/64.0-0.5)/255.0, (55/64.0-0.5)/255.0, (23/64.0-0.5)/255.0, (61/64.0-0.5)/255.0, (29/64.0-0.5)/255.0, (53/64.0-0.5)/255.0, (21/64.0-0.5)/255.0} };

float3 FBDither(const float3 color, const int2 pos)
{
   return color + bayer_dither_pattern[pos.x%8][pos.y%8];
}*/

float3 FBColorGrade(float3 color)
{
   if(!color_grade)
       return color;

   color.xy = color.xy*(15.0/16.0) + 1.0/32.0; // assumes 16x16x16 resolution flattened to 256x16 texture
   color.z *= 15.0;

   const float x = (color.x + floor(color.z))/16.0;
   const float3 lut1 = tex2Dlod(texSampler6, float4(x,          color.y, 0.,0.)).xyz; // two lookups to blend/lerp between blue 2D regions
   const float3 lut2 = tex2Dlod(texSampler6, float4(x+1.0/16.0, color.y, 0.,0.)).xyz;
   return lerp(lut1,lut2, frac(color.z));
}

#include "FXAAStereoAO.fxh"

//
//
//

VS_OUTPUT_2D vs_main_no_trafo (float4 vPosition  : POSITION0,  
                               float2 tc         : TEXCOORD0)
{ 
   VS_OUTPUT_2D Out;

   Out.pos = float4(vPosition.xy, 0.0,1.0);
   Out.tex0 = tc;
   
   return Out; 
}

//
// PS functions
//

float4 ps_main_fb_tonemap( in VS_OUTPUT_2D IN) : COLOR
{
    //!! const float depth0 = tex2Dlod(texSamplerDepth, float4(u, 0.,0.)).x;
    //!! if((depth0 == 1.0) || (depth0 == 0.0)) //!! early out if depth too large (=BG) or too small (=DMD,etc -> retweak render options (depth write on), otherwise also screwup with stereo)
    //!!   return float4(tex2Dlod(texSampler5, float4(IN.tex0, 0.,0.)).xyz, 1.0);

    const float3 result = FBToneMap(tex2Dlod(texSampler5, float4(IN.tex0, 0.,0.)).xyz) + tex2Dlod(texSamplerBloom, float4(IN.tex0, 0.,0.)).xyz; //!! offset?
    return float4(FBColorGrade(FBGamma(saturate(result))), 1.0);
}

float4 ps_main_fb_bloom( in VS_OUTPUT_2D IN) : COLOR
{
    // collect clipped contribution of the 3x3 texels (via box blur (offset: 0.25*pixel=w_h_height.xy*0.5), NOT gaussian, as this is wrong) from original FB
    const float3 result = (tex2Dlod(texSampler5, float4(IN.tex0-w_h_height.xy*0.5, 0.,0.)).xyz
                        +  tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*2.5, 0.,0.)).xyz
                        +  tex2Dlod(texSampler5, float4(IN.tex0+float2(w_h_height.x*2.5,-w_h_height.y*0.5), 0.,0.)).xyz
                        +  tex2Dlod(texSampler5, float4(IN.tex0+float2(-w_h_height.x*0.5,w_h_height.y*2.5), 0.,0.)).xyz)*0.25; //!! offset for useAA?
    return float4(max(FBToneMap(result)-float3(1.,1.,1.), float3(0.,0.,0.)), 1.0);
}

float4 ps_main_fb_AO(in VS_OUTPUT_2D IN) : COLOR
{
	const float3 result = tex2Dlod(texSampler3, float4(IN.tex0/*-w_h_height.xy*/, 0., 0.)).x; // omitting the shift blurs over 2x2 window
	return float4(FBGamma(saturate(result)), 1.0);
}

float4 ps_main_fb_tonemap_AO( in VS_OUTPUT_2D IN) : COLOR
{
    const float3 result = FBToneMap(tex2Dlod(texSampler5, float4(IN.tex0, 0.,0.)).xyz) // moving AO before tonemap does not really change the look
           * tex2Dlod(texSampler3, float4(IN.tex0/*-w_h_height.xy*/, 0.,0.)).x // omitting the shift blurs over 2x2 window
           + tex2Dlod(texSamplerBloom, float4(IN.tex0, 0.,0.)).xyz;  //!! offset?
    return float4(FBColorGrade(FBGamma(saturate(result))), 1.0);
}

float4 ps_main_fb_tonemap_AO_static(in VS_OUTPUT_2D IN) : COLOR
{
	const float3 result = tex2Dlod(texSampler5, float4(IN.tex0, 0., 0.)).xyz
	* tex2Dlod(texSampler3, float4(IN.tex0/*-w_h_height.xy*/, 0., 0.)).x; // omitting the shift blurs over 2x2 window
	return float4(result, 1.0);
}

float4 ps_main_fb_tonemap_no_filter( in VS_OUTPUT_2D IN) : COLOR
{
    const float3 result = FBToneMap(tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy, 0.,0.)).xyz) + tex2Dlod(texSamplerBloom, float4(IN.tex0, 0.,0.)).xyz; //!! offset?
    return float4(/*FBDither(*/FBColorGrade(FBGamma(saturate(result))),/*IN.tex0*w_h_height.zw),*/ 1.0);
}

float4 ps_main_fb_tonemap_AO_no_filter( in VS_OUTPUT_2D IN) : COLOR
{
    const float3 result = FBToneMap(tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy, 0.,0.)).xyz) // moving AO before tonemap does not really change the look
           * tex2Dlod(texSampler3, float4(IN.tex0/*-w_h_height.xy*/, 0.,0.)).x // omitting the shift blurs over 2x2 window
	   + tex2Dlod(texSamplerBloom, float4(IN.tex0, 0.,0.)).xyz;  //!! offset?
    return float4(FBColorGrade(FBGamma(saturate(result))), 1.0);
}

float4 ps_main_fb_tonemap_AO_no_filter_static(in VS_OUTPUT_2D IN) : COLOR
{
	const float3 result = tex2Dlod(texSampler4, float4(IN.tex0 + w_h_height.xy, 0., 0.)).xyz
	* tex2Dlod(texSampler3, float4(IN.tex0/*-w_h_height.xy*/, 0., 0.)).x; // omitting the shift blurs over 2x2 window
	return float4(result, 1.0);
}

//
// Bloom (9x9)
//

#if 0 // full or abusing lerp
static const float offset[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
static const float weight[5] = { 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };

float4 ps_main_fb_bloom_horiz( in VS_OUTPUT_2D IN) : COLOR
{
    float3 result = tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.)).xyz*weight[0];
    [unroll] for(int i = 1; i < 5; ++i)
    {
        result += tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5+float2(w_h_height.x*offset[i],0.0), 0.,0.)).xyz*weight[i];
        result += tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5-float2(w_h_height.x*offset[i],0.0), 0.,0.)).xyz*weight[i];
    }
    return float4(result, 1.0);
}

float4 ps_main_fb_bloom_vert( in VS_OUTPUT_2D IN) : COLOR
{
    float3 result = tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.)).xyz*weight[0];
    [unroll] for(int i = 1; i < 5; ++i)
    {
        result += tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5+float2(0.0,w_h_height.y*offset[i]), 0.,0.)).xyz*weight[i];
        result += tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5-float2(0.0,w_h_height.y*offset[i]), 0.,0.)).xyz*weight[i];
    }
    return float4(result*w_h_height.z, 1.0);
}

#else

static const float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
//13: 0.0,1.411764705882353,3.2941176470588234,5.176470588235294
static const float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };
//13: 0.1964825501511404,0.2969069646728344,0.09447039785044732,0.010381362401148057

float4 ps_main_fb_bloom_horiz( in VS_OUTPUT_2D IN) : COLOR
{
    float3 result = tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.)).xyz*weight[0];
    [unroll] for(int i = 1; i < 3; ++i)
    {
        result += tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(w_h_height.x*offset[i],0.0), 0.,0.)).xyz*weight[i];
        result += tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(w_h_height.x*offset[i],0.0), 0.,0.)).xyz*weight[i];
    }
    return float4(result, 1.0);
}

float4 ps_main_fb_bloom_vert( in VS_OUTPUT_2D IN) : COLOR
{
    float3 result = tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.)).xyz*weight[0];
    [unroll] for(int i = 1; i < 3; ++i)
    {
        result += tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(0.0,w_h_height.y*offset[i]), 0.,0.)).xyz*weight[i];
        result += tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(0.0,w_h_height.y*offset[i]), 0.,0.)).xyz*weight[i];
    }
    return float4(result*w_h_height.z, 1.0);
}
#endif

// mirror
float4 ps_main_fb_mirror(in VS_OUTPUT_2D IN) : COLOR
{
   return tex2D(texSamplerMirror, IN.tex0) * mirrorFactor;
}


//
// Techniques
//

/*technique normals // generate normals into 8bit RGB
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_normals();
   } 
}*/

technique AO
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_ao();
   } 
}

technique stereo
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_stereo();
   } 
}

technique NFAA
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_nfaa();
	}
}

technique FXAA1
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fxaa1();
   } 
}

technique FXAA2
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fxaa2();
   } 
}

technique FXAA3
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fxaa3();
   } 
}

technique fb_tonemap
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_tonemap();
   } 
}

technique fb_bloom
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_bloom();
   } 
}

technique fb_AO
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_fb_AO();
	}
}

technique fb_tonemap_AO
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_tonemap_AO();
   } 
}

technique fb_tonemap_AO_static
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_fb_tonemap_AO_static();
	}
}

technique fb_tonemap_no_filter
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_tonemap_no_filter();
   } 
}

technique fb_tonemap_AO_no_filter
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_tonemap_AO_no_filter();

	  //!! SRGBWriteEnable = true;
   } 
}

technique fb_tonemap_AO_no_filter_static
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_fb_tonemap_AO_no_filter_static();
	}
}

technique fb_bloom_horiz
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_bloom_horiz();
   } 
}

technique fb_bloom_vert
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
	  PixelShader = compile ps_3_0 ps_main_fb_bloom_vert();
   } 
}

technique fb_mirror
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader = compile ps_3_0 ps_main_fb_mirror();
   }
}
