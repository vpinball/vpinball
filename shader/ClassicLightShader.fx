#ifdef SEPARATE_CLASSICLIGHTSHADER
#define NUM_BALL_LIGHTS 0 // just to avoid having too much constant mem allocated

#include "Helpers.fxh"

// transformation matrices
float4x4 matWorldViewProj : WORLDVIEWPROJ;
float4x4 matWorldView     : WORLDVIEW;
float3x4 matWorldViewInverseTranspose;
float4x3 matView;
//float4x4 matViewInverseInverseTranspose; // matView used instead and multiplied from other side

texture Texture0; // base texture
texture Texture1; // envmap
texture Texture2; // envmap radiance
 
sampler2D texSampler0 : TEXUNIT0 = sampler_state // base texture
{
	Texture	  = (Texture0);
    //MIPFILTER = LINEAR; //!! HACK: not set here as user can choose to override trilinear by anisotropic
    //MAGFILTER = LINEAR;
    //MINFILTER = LINEAR;
	//ADDRESSU  = Wrap; //!! ?
	//ADDRESSV  = Wrap;
	//!! SRGBTexture = true;
};

sampler2D texSampler1 : TEXUNIT1 = sampler_state // environment
{
	Texture	  = (Texture1);
    MIPFILTER = LINEAR; //!! ?
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Wrap;
	ADDRESSV  = Wrap;
};

sampler2D texSampler2 : TEXUNIT2 = sampler_state // diffuse environment contribution/radiance
{
	Texture	  = (Texture2);
    MIPFILTER = NONE;
    MAGFILTER = LINEAR;
    MINFILTER = LINEAR;
	ADDRESSU  = Wrap;
	ADDRESSV  = Wrap;
};

#include "Material.fxh"

float3 cGlossy_ImageLerp; // actually doesn't feature image lerp
float3 cClearcoat_EdgeAlpha; // actually doesn't feature edge-alpha
//!! No value is under 0.02
//!! Non-metals value are un-intuitively low: 0.02-0.08
//!! Gemstones are 0.05-0.17
//!! Metals have high specular reflectance:  0.5-1.0

//float  fAlphaTestValue;

bool hdrEnvTextures;
#endif

float4 lightColor_intensity;
float4 lightColor2_falloff_power;
float4 lightCenter_maxRange;
float2 imageBackglassMode; // actually bool2

bool hdrTexture0;

struct VS_LIGHT_OUTPUT 
{ 
   float4 pos           : POSITION;
   float2 tex0          : TEXCOORD0; 
   float3 worldPos      : TEXCOORD1;
   float3 tablePos      : TEXCOORD2; 
   float3 normal        : TEXCOORD3;
};

VS_LIGHT_OUTPUT vs_light_main (float4 vPosition : POSITION0,  
                               float3 vNormal   : NORMAL0,  
                               float2 tc        : TEXCOORD0) 
{ 
   VS_LIGHT_OUTPUT Out;

   // trafo all into worldview space (as most of the weird trafos happen in view, world is identity so far)
   const float3 P = mul(vPosition, matWorldView).xyz;
   const float3 N = normalize(mul(vNormal, matWorldViewInverseTranspose).xyz);

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;
   Out.tablePos = vPosition.xyz;
   Out.worldPos = P;
   Out.normal = N;
   
   return Out; 
}

float4 PS_LightWithTexel(in VS_LIGHT_OUTPUT IN, uniform bool is_metal) : COLOR
{
    float4 pixel = tex2D(texSampler0, IN.tex0); //!! IN.tex0 abused in backglass mode
    if(!hdrTexture0)
        pixel.xyz = InvGamma(pixel.xyz);

    float4 color;
    // no lighting if HUD vertices or passthrough mode
    [branch] if(imageBackglassMode.x != 0. || imageBackglassMode.y != 0.)
        color = pixel;
    else
    {
        pixel.xyz = saturate(pixel.xyz); // could be HDR
        const float3 diffuse  = pixel.xyz*cBase_Alpha.xyz;
        const float3 glossy   = is_metal ? diffuse : pixel.xyz*cGlossy_ImageLerp.xyz*0.08; //!! use AO for glossy? specular?
        const float3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
        const float  edge     = is_metal ? 1.0 : Roughness_WrapL_Edge_Thickness.z;

        color.xyz = lightLoop(IN.worldPos, normalize(IN.normal), normalize(/*camera=0,0,0,1*/-IN.worldPos), diffuse, glossy, specular, edge, true, is_metal); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
        color.a = pixel.a;
    }
    color.a *= cBase_Alpha.a;

	[branch] if(lightColor_intensity.w != 0.0)
    {
        const float len = length(lightCenter_maxRange.xyz - (imageBackglassMode.y == 0. ? IN.tablePos : float3(IN.tex0,0.0))) * lightCenter_maxRange.w;
        const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
        const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
        color += float4(lcolor*(atten*lightColor_intensity.w),
                        saturate(atten*lightColor_intensity.w));
        color = OverlayHDR(pixel, color); // could be HDR //!! have mode to choose: if simple mode picked and surface images match then can skip lighting texel above and JUST alpha blend with this here
        color = ScreenHDR(pixel, color);
    }

    return color;
}

float4 PS_LightWithoutTexel(in VS_LIGHT_OUTPUT IN, uniform bool is_metal) : COLOR
{
    float4 result = float4(0.0, 0.0, 0.0, 0.0);
    [branch] if (lightColor_intensity.w != 0.0)
    {
        const float len = length(lightCenter_maxRange.xyz - (imageBackglassMode.y == 0. ? IN.tablePos : float3(IN.tex0,0.0))) * lightCenter_maxRange.w;
        const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
        const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
        result.xyz = lcolor*(atten*lightColor_intensity.w);
        result.a = saturate(atten*lightColor_intensity.w);
    }

    float4 color;
    // no lighting if HUD vertices or passthrough mode
    [branch] if(imageBackglassMode.x != 0. || imageBackglassMode.y != 0.)
        color.xyz = lightColor_intensity.xyz;
    else
    {
        const float3 diffuse  = lightColor_intensity.xyz*cBase_Alpha.xyz;
        const float3 glossy   = is_metal ? diffuse : lightColor_intensity.xyz*cGlossy_ImageLerp.xyz*0.08;
        const float3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
        const float  edge     = is_metal ? 1.0 : Roughness_WrapL_Edge_Thickness.z;

        color.xyz = lightLoop(IN.worldPos, normalize(IN.normal), normalize(/*camera=0,0,0,1*/-IN.worldPos), diffuse, glossy, specular, edge, true, is_metal); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
    }
    color.a = cBase_Alpha.a;
    
    return color+result;
}

technique light_with_texture_isMetal
{ 
   pass P0 
   { 
//       AlphaBlendEnable = true;
//       SrcBlend = One;
//       DestBlend = One;
       VertexShader = compile vs_3_0 vs_light_main();
	  PixelShader = compile ps_3_0 PS_LightWithTexel(1);
   } 
}

technique light_with_texture_isNotMetal
{ 
   pass P0 
   { 
//       AlphaBlendEnable = true;
//       SrcBlend = One;
//       DestBlend = One;
       VertexShader = compile vs_3_0 vs_light_main();
	  PixelShader = compile ps_3_0 PS_LightWithTexel(0);
   } 
}

technique light_without_texture_isMetal
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_light_main(); 
	  PixelShader = compile ps_3_0 PS_LightWithoutTexel(1);
   } 
}

technique light_without_texture_isNotMetal
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_light_main(); 
	  PixelShader = compile ps_3_0 PS_LightWithoutTexel(0);
   } 
}
