#ifdef SEPARATE_CLASSICLIGHTSHADER
#define NUM_BALL_LIGHTS 0 // just to avoid having too much constant mem allocated

#include "Helpers.fxh"

// transformation matrices
float4x4 matWorldViewProj : WORLDVIEWPROJ;
float4x4 matWorldView     : WORLDVIEW;
float4x4 matWorldViewInverseTranspose;
float4x4 matView;
float4x4 matViewInverse;

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

float3 cGlossy;
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
   const float3 N = normalize(mul(float4(vNormal,0.0), matWorldViewInverseTranspose).xyz);

   Out.pos = mul(vPosition, matWorldViewProj);
   Out.tex0 = tc;
   Out.tablePos = vPosition.xyz;
   Out.worldPos = P;
   Out.normal = N;
   
   return Out; 
}

float4 PS_LightWithTexel(in VS_LIGHT_OUTPUT IN) : COLOR
{	
    float4 pixel = tex2D(texSampler0, IN.tex0); //!! IN.tex0 abused in backglass mode
    if(!hdrTexture0)
        pixel.xyz = InvGamma(pixel.xyz);

    float4 color;
	// no lighting if HUD vertices or passthrough mode
    if(imageBackglassMode.x != 0. || imageBackglassMode.y != 0.)
        color = pixel;
    else
	{
        pixel.xyz = saturate(pixel.xyz); // could be HDR
        const float3 diffuse = pixel.xyz*cBase_Alpha.xyz;
        const float3 glossy = (Roughness_WrapL_Edge_IsMetal.w != 0.0) ? diffuse : pixel.xyz*cGlossy*0.08; //!! use AO for glossy? specular?
        const float3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
        const float edge = (Roughness_WrapL_Edge_IsMetal.w != 0.0) ? 1.0 : Roughness_WrapL_Edge_IsMetal.z;

	    color.xyz = lightLoop(IN.worldPos, normalize(IN.normal), normalize(/*camera=0,0,0,1*/-IN.worldPos), diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
		color.a = pixel.a;
    }
    color.a *= cBase_Alpha.a;

    if ( lightColor_intensity.w!=0.0 )
    {
        const float len = length(lightCenter_maxRange.xyz - (imageBackglassMode.y == 0. ? IN.tablePos : float3(IN.tex0,0.0))) * lightCenter_maxRange.w;
        const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
        const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
        color += float4(lcolor*(atten*lightColor_intensity.w),
                        saturate(atten*lightColor_intensity.w));
        pixel.xyz = saturate(pixel.xyz); // could be HDR
        color.xyz = saturate(color.xyz); // because of overlay and screen restrictions
        color = Overlay(pixel, color);
        color = Screen(pixel, color);
    }

    return color;
}

float4 PS_LightWithoutTexel(in VS_LIGHT_OUTPUT IN) : COLOR
{
    float4 result = float4(0.0, 0.0, 0.0, 0.0);
    if (lightColor_intensity.w != 0.0)
    {
        const float len = length(lightCenter_maxRange.xyz - (imageBackglassMode.y == 0. ? IN.tablePos : float3(IN.tex0,0.0))) * lightCenter_maxRange.w;
        const float atten = pow(1.0 - saturate(len), lightColor2_falloff_power.w);
        const float3 lcolor = lerp(lightColor2_falloff_power.xyz, lightColor_intensity.xyz, sqrt(len));
        result.xyz = lcolor*(atten*lightColor_intensity.w);
        result.a = saturate(atten*lightColor_intensity.w);
    }

	float4 color;
	// no lighting if HUD vertices or passthrough mode
    if(imageBackglassMode.x != 0. || imageBackglassMode.y != 0.)
        color.xyz = lightColor_intensity.xyz;
    else
	{
	    const float3 diffuse  = lightColor_intensity.xyz*cBase_Alpha.xyz;
        const float3 glossy   = (Roughness_WrapL_Edge_IsMetal.w != 0.0) ? diffuse : lightColor_intensity.xyz*cGlossy*0.08;
        const float3 specular = cClearcoat_EdgeAlpha.xyz*0.08;
	    const float edge = (Roughness_WrapL_Edge_IsMetal.w != 0.0) ? 1.0 : Roughness_WrapL_Edge_IsMetal.z;

	    color.xyz = lightLoop(IN.worldPos, normalize(IN.normal), normalize(/*camera=0,0,0,1*/-IN.worldPos), diffuse, glossy, specular, edge); //!! have a "real" view vector instead that mustn't assume that viewer is directly in front of monitor? (e.g. cab setup) -> viewer is always relative to playfield and/or user definable
	}
    color.a = cBase_Alpha.a;
    
    return color+result;
}

technique light_with_texture
{ 
   pass P0 
   { 
//       AlphaBlendEnable = true;
//       SrcBlend = One;
//       DestBlend = One;
       VertexShader = compile vs_3_0 vs_light_main();
	  PixelShader = compile ps_3_0 PS_LightWithTexel();
   } 
}

technique light_without_texture
{ 
   pass P0 
   { 
      VertexShader = compile vs_3_0 vs_light_main(); 
	  PixelShader = compile ps_3_0 PS_LightWithoutTexel();
   } 
}
