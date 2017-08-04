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
float4 ps_main_fb_bloom_horiz9x9( in VS_OUTPUT_2D IN) : COLOR
{
    const float offset9x9[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const float weight9x9[5] = { 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };
    float3 result = tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.)).xyz*weight9x9[0];
    [unroll] for(int i = 1; i < 5; ++i)
    {
        result += tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5+float2(w_h_height.x*offset9x9[i],0.0), 0.,0.)).xyz*weight9x9[i];
        result += tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5-float2(w_h_height.x*offset9x9[i],0.0), 0.,0.)).xyz*weight9x9[i];
    }
    return float4(result, 1.0);
}

float4 ps_main_fb_bloom_vert9x9(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset9x9[5] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    const float weight9x9[5] = { 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };
    float3 result = tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.)).xyz*weight9x9[0];
    [unroll] for(int i = 1; i < 5; ++i)
    {
        result += tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5+float2(0.0,w_h_height.y*offset9x9[i]), 0.,0.)).xyz*weight9x9[i];
        result += tex2Dlod(texSampler4, float4(IN.tex0+w_h_height.xy*0.5-float2(0.0,w_h_height.y*offset9x9[i]), 0.,0.)).xyz*weight9x9[i];
    }
    return float4(result*w_h_height.z, 1.0);
}

#else

float4 ps_main_fb_bloom_horiz9x9(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset9x9[3] = { 0.0, 1.3846153846, 3.2307692308 };
    const float weight9x9[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };
    float3 result = tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.)).xyz*weight9x9[0];
    [unroll] for(int i = 1; i < 3; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(w_h_height.x*offset9x9[i],0.0), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(w_h_height.x*offset9x9[i],0.0), 0.,0.)).xyz)*weight9x9[i];

    return float4(result, 1.0);
}

float4 ps_main_fb_bloom_vert9x9(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset9x9[3] = { 0.0, 1.3846153846, 3.2307692308 };
    const float weight9x9[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };
    float3 result = tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.)).xyz*weight9x9[0];
    [unroll] for(int i = 1; i < 3; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(0.0,w_h_height.y*offset9x9[i]), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(0.0,w_h_height.y*offset9x9[i]), 0.,0.)).xyz)*weight9x9[i];

    return float4(result*w_h_height.z, 1.0);
}

#if 0
float4 ps_main_fb_bloom_horiz9x9_4(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset9x9[3] = { 0.0, 1.3846153846, 3.2307692308 };
    const float weight9x9[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };
    float4 result = tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.))*weight9x9[0];
    [unroll] for (int i = 1; i < 3; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(w_h_height.x*offset9x9[i],0.0), 0.,0.))
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(w_h_height.x*offset9x9[i],0.0), 0.,0.)))*weight9x9[i];

    return result;
}

float4 ps_main_fb_bloom_vert9x9_4(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset9x9[3] = { 0.0, 1.3846153846, 3.2307692308 };
    const float weight9x9[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };
    float4 result = tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5, 0.,0.))*weight9x9[0];
    [unroll] for (int i = 1; i < 3; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(0.0,w_h_height.y*offset9x9[i]), 0.,0.))
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(0.0,w_h_height.y*offset9x9[i]), 0.,0.)))*weight9x9[i];

    return result*w_h_height.z;
}


//const float offset[4] = { 0.0, 1.411764705882353, 3.2941176470588234, 5.176470588235294 }; //13
//const float weight[4] = { 0.1964825501511404, 0.2969069646728344, 0.09447039785044732, 0.010381362401148057 }; //13

//const float offset[2] = { 0.53473, 2.05896 }; //7 (no center!)
//const float weight[2] = { 0.45134, 0.04866 }; //7 (no center!)
//const float offset[4] = { 0.64417, 2.37795, 4.28970, 6.21493 }; //15 (no center!)
//const float weight[4] = { 0.25044, 0.19233, 0.05095, 0.00628 }; //15 (no center!)
//const float offset[6] = { 0.65769, 2.45001, 4.41069, 6.37247, 8.33578, 10.30098 }; //23 (no center!)
//const float weight[6] = { 0.16526, 0.17520, 0.10103, 0.04252, 0.01306, 0.00293 }; //23 (no center!)

//const float offset11x11h[3] = { 0.59804, 2.18553, 4.06521 }; //no center!
//const float weight11x11h[3] = { 0.38173, 0.11538, 0.00289 }; //no center!

float4 ps_main_fb_bloom_horiz11x11(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset11x11[3] = { 0.62195, 2.27357, 4.14706 }; //no center!
    const float weight11x11[3] = { 0.32993, 0.15722, 0.01285 }; //no center!
    float3 result = float3(0.0, 0.0, 0.0);
    [unroll] for(int i = 0; i < 3; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(w_h_height.x*offset11x11[i],0.0), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(w_h_height.x*offset11x11[i],0.0), 0.,0.)).xyz)*weight11x11[i];

    return float4(result, 1.0);
}

float4 ps_main_fb_bloom_vert11x11(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset11x11[3] = { 0.62195, 2.27357, 4.14706 }; //no center!
    const float weight11x11[3] = { 0.32993, 0.15722, 0.01285 }; //no center!
    float3 result = float3(0.0, 0.0, 0.0);
    [unroll] for(int i = 0; i < 3; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(0.0,w_h_height.y*offset11x11[i]), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(0.0,w_h_height.y*offset11x11[i]), 0.,0.)).xyz)*weight11x11[i];

    return float4(result*w_h_height.z, 1.0);
}
#endif

float4 ps_main_fb_bloom_horiz19x19(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset19x19[5] = { 0.65323, 2.42572, 4.36847, 6.31470, 8.26547 }; //no center!
    const float weight19x19[5] = { 0.19923, 0.18937, 0.08396, 0.02337, 0.00408 }; //no center!
    float3 result = float3(0.0, 0.0, 0.0);
    [unroll] for(int i = 0; i < 5; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(w_h_height.x*offset19x19[i],0.0), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(w_h_height.x*offset19x19[i],0.0), 0.,0.)).xyz)*weight19x19[i];

    return float4(result, 1.0);
}

float4 ps_main_fb_bloom_vert19x19(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset19x19[5] = { 0.65323, 2.42572, 4.36847, 6.31470, 8.26547 }; //no center!
    const float weight19x19[5] = { 0.19923, 0.18937, 0.08396, 0.02337, 0.00408 }; //no center!
    float3 result = float3(0.0, 0.0, 0.0);
    [unroll] for(int i = 0; i < 5; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(0.0,w_h_height.y*offset19x19[i]), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(0.0,w_h_height.y*offset19x19[i]), 0.,0.)).xyz)*weight19x19[i];

    return float4(result*w_h_height.z, 1.0);
}

//const float offset19x19h[5] = { 0.64625, 2.38872, 4.30686, 6.23559, 8.17666 };
//const float weight19x19h[5] = { 0.23996, 0.19335, 0.05753, 0.00853, 0.00063 };
//const float offset19x19h[5] = { 0.63232, 2.31979, 4.20448, 6.12322, 8.07135 };
//const float weight19x19h[5] = { 0.29809, 0.17619, 0.02462, 0.00109, 0.00002 };

float4 ps_main_fb_bloom_horiz19x19h(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset19x19h[5] = { 0.63918, 2.35282, 4.25124, 6.17117, 8.11278 }; //no center!
    const float weight19x19h[5] = { 0.27233, 0.18690, 0.03767, 0.00301, 0.00009 }; //no center!
    float3 result = float3(0.0, 0.0, 0.0);
    [unroll] for(int i = 0; i < 5; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(w_h_height.x*offset19x19h[i],0.0), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(w_h_height.x*offset19x19h[i],0.0), 0.,0.)).xyz)*weight19x19h[i];

    return float4(result, 1.0);
}

float4 ps_main_fb_bloom_vert19x19h(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset19x19h[5] = { 0.63918, 2.35282, 4.25124, 6.17117, 8.11278 }; //no center!
    const float weight19x19h[5] = { 0.27233, 0.18690, 0.03767, 0.00301, 0.00009 }; //no center!
    float3 result = float3(0.0, 0.0, 0.0);
    [unroll] for(int i = 0; i < 5; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(0.0,w_h_height.y*offset19x19h[i]), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(0.0,w_h_height.y*offset19x19h[i]), 0.,0.)).xyz)*weight19x19h[i];

    return float4(result*w_h_height.z, 1.0);
}

#if 0
float4 ps_main_fb_bloom_horiz27x27(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset27x27[7] = { 0.66025, 2.46412, 4.43566, 6.40762, 8.38017, 10.35347, 12.32765 }; //no center!
    const float weight27x27[7] = { 0.14096, 0.15932, 0.10714, 0.05743, 0.02454, 0.00835, 0.00227 };   //no center!
    float3 result = float3(0.0, 0.0, 0.0);
    [unroll] for(int i = 0; i < 7; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(w_h_height.x*offset27x27[i],0.0), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(w_h_height.x*offset27x27[i],0.0), 0.,0.)).xyz)*weight27x27[i];

    return float4(result, 1.0);
}

float4 ps_main_fb_bloom_vert27x27(in VS_OUTPUT_2D IN) : COLOR
{
    const float offset27x27[7] = { 0.66025, 2.46412, 4.43566, 6.40762, 8.38017, 10.35347, 12.32765 }; //no center!
    const float weight27x27[7] = { 0.14096, 0.15932, 0.10714, 0.05743, 0.02454, 0.00835, 0.00227 };   //no center!
    float3 result = float3(0.0, 0.0, 0.0);
    [unroll] for(int i = 0; i < 7; ++i)
        result += (tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5+float2(0.0,w_h_height.y*offset27x27[i]), 0.,0.)).xyz
                  +tex2Dlod(texSampler5, float4(IN.tex0+w_h_height.xy*0.5-float2(0.0,w_h_height.y*offset27x27[i]), 0.,0.)).xyz)*weight27x27[i];

    return float4(result*w_h_height.z, 1.0);
}
#endif
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

technique DLAA_edge
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_dlaa_edge();
	}
}

technique DLAA
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_dlaa();
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

// All Bloom variants:

technique fb_bloom_horiz9x9
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader = compile ps_3_0 ps_main_fb_bloom_horiz9x9();
   }
}

technique fb_bloom_vert9x9
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader = compile ps_3_0 ps_main_fb_bloom_vert9x9();
   }
}

#if 0
technique fb_bloom_horiz9x9_4
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader = compile ps_3_0 ps_main_fb_bloom_horiz9x9_4();
   }
}

technique fb_bloom_vert9x9_4
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader = compile ps_3_0 ps_main_fb_bloom_vert9x9_4();
   }
}

technique fb_bloom_horiz11x11
{ 
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader = compile ps_3_0 ps_main_fb_bloom_horiz11x11();
   }
}

technique fb_bloom_vert11x11
{ 
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader = compile ps_3_0 ps_main_fb_bloom_vert11x11();
   }
}
#endif

technique fb_bloom_horiz19x19
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_fb_bloom_horiz19x19();
	}
}

technique fb_bloom_vert19x19
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_fb_bloom_vert19x19();
	}
}

technique fb_bloom_horiz19x19h
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_fb_bloom_horiz19x19h();
	}
}

technique fb_bloom_vert19x19h
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_fb_bloom_vert19x19h();
	}
}

#if 0
technique fb_bloom_horiz27x27
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_fb_bloom_horiz27x27();
	}
}

technique fb_bloom_vert27x27
{
	pass P0
	{
		VertexShader = compile vs_3_0 vs_main_no_trafo();
		PixelShader = compile ps_3_0 ps_main_fb_bloom_vert27x27();
	}
}
#endif

//

technique fb_mirror
{
   pass P0
   {
      VertexShader = compile vs_3_0 vs_main_no_trafo();
      PixelShader = compile ps_3_0 ps_main_fb_mirror();
   }
}

#include "SMAA.fxh"
